#include "client.h"
#include "buffer.h"
#include "macros.h"
#include "log.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CLIENT_INITIAL_INPUT_CAPACITY 1024
#define CLIENT_INITIAL_OUTPUT_CAPACITY 2048

// Telnet command codes
#define TELNET_IAC   255  // "Interpret As Command"
#define TELNET_DONT  254
#define TELNET_DO    253
#define TELNET_WONT  252
#define TELNET_WILL  251
#define TELNET_SB    250  // Begin subnegotiation
#define TELNET_SE    240  // End subnegotiation

static bool client_process_input_byte(Client *client, unsigned char byte, char *out_char);

Client *client_create(int socket_fd, struct sockaddr_in *addr) {
    Client *client = malloc(sizeof(Client));
    if (!client) {
        return NULL;
    }

    client->socket_fd = socket_fd;
    client->address = *addr;
    inet_ntop(AF_INET, &(addr->sin_addr), client->ip_string, sizeof(client->ip_string));
    client->connected = true;

    // Initialize input state
    client->input_length = 0;
    client->input_line_end = 0;
    client->input_discarding = false;

    // Initialize output buffer
    client->output = buffer_create(CLIENT_INITIAL_OUTPUT_CAPACITY);
    if (!client->output) {
        free(client);
        return NULL;
    }

    // Default state values
    client->name[0] = '\0';
    client->user_data = NULL;
    client->state = CLIENT_STATE_MENU;
    client->input_handler = NULL;

    client->actor = NULL;

    return client;
}

void client_destroy(Client *client) {
    if (!client) return;

    if (client->actor) {
        CHECK_MSG(client->actor->client == client,
            "Client %s is linked to actor %u, but actor->client != client",
            client->ip_string, client->actor->id);

        client->actor->client = NULL;
        client->actor = NULL;
    }

    close(client->socket_fd);
    buffer_destroy(client->output);
    free(client);
}

bool client_read(Client *client) {
    if (client->input_line_end != 0) {
        return true; // Already have a full line
    }

    char temp[CLIENT_MAX_LINE + 1]; // safe size for one full line (+ \0)
    size_t read_limit;

    if (client->input_discarding) {
        read_limit = sizeof(temp);
    } else {
        size_t space_left = CLIENT_INPUT_BUFFER_SIZE - client->input_length;
        read_limit = space_left > sizeof(temp) ? sizeof(temp) : space_left;
        if (read_limit == 0) {
            client->input_length = 0;
            client->input_discarding = true;
            read_limit = sizeof(temp);
        }
    }

    ssize_t bytes = read(client->socket_fd, temp, read_limit);
    if (bytes <= 0) {
        client->connected = false;
        return false;
    }

    for (ssize_t i = 0; i < bytes; ++i) {
        char c;
        if (!client_process_input_byte(client, (unsigned char)temp[i], &c)) {
            continue; // not a normal input character
        }

        if (c == '\r') continue;

        if (c == '\n') {
            if (!client->input_discarding) {
                // Trim trailing
                while (client->input_length > 0 &&
                       isspace((unsigned char)client->input_buffer[client->input_length - 1])) {
                    client->input_length--;
                }

                // Trim leading
                size_t leading = 0;
                while (leading < client->input_length &&
                       isspace((unsigned char)client->input_buffer[leading])) {
                    leading++;
                }

                if (leading > 0 && leading < client->input_length) {
                    memmove(client->input_buffer,
                            client->input_buffer + leading,
                            client->input_length - leading);
                }

                client->input_length -= leading;
                client->input_buffer[client->input_length] = '\0';
                client->input_line_end = client->input_length;
                return true; // One line complete, ready for processing
            } else {
                // Discarded line ends — reset and prepare for next
                client->input_length = 0;
                client->input_discarding = false;
            }
            continue;
        }

        if (client->input_discarding) {
            continue;
        }

        if (client->input_length < CLIENT_MAX_LINE) {
            client->input_buffer[client->input_length++] = c;
        } else {
            // Line is too long, discard until '\n'
            client->input_length = 0;
            client->input_discarding = true;
        }
    }

    return true;
}

static bool client_process_input_byte(Client *client, unsigned char byte, char *out_char) {
    switch (client->telnet_state) {
        case TELNET_STATE_DATA:
            if (byte == TELNET_IAC) {
                client->telnet_state = TELNET_STATE_IAC;
                return false;
            } else {
                *out_char = byte;
                return true;
            }

        case TELNET_STATE_IAC:
            if (byte == TELNET_IAC) {
                client->telnet_state = TELNET_STATE_DATA;
                *out_char = TELNET_IAC;
                return true;
            } else if (byte == TELNET_DO || byte == TELNET_DONT ||
                       byte == TELNET_WILL || byte == TELNET_WONT) {
                client->telnet_command = byte;
                client->telnet_state = TELNET_STATE_COMMAND;
            } else if (byte == TELNET_SB) {
                client->telnet_state = TELNET_STATE_SB;
            } else {
                client->telnet_state = TELNET_STATE_DATA;
            }
            return false;

        case TELNET_STATE_COMMAND:
            log_trace("Telnet command %u %u", client->telnet_command, byte);
            client->telnet_state = TELNET_STATE_DATA;
            return false;

        case TELNET_STATE_SB:
            client->telnet_state = TELNET_STATE_SB_DATA;
            return false;

        case TELNET_STATE_SB_DATA:
            if (byte == TELNET_IAC) {
                client->telnet_state = TELNET_STATE_SB_IAC;
            }
            return false;

        case TELNET_STATE_SB_IAC:
            if (byte == TELNET_SE) {
                client->telnet_state = TELNET_STATE_DATA;
            } else {
                client->telnet_state = TELNET_STATE_SB_DATA;
            }
            return false;
    }

    return false;
}


bool client_write(Client *client, const char *text) {
    return buffer_append(client->output, text, strlen(text));
}

bool client_writeln(Client *client, const char *text) {
    return buffer_append(client->output, text, strlen(text)) &&
           buffer_append(client->output, "\r\n", 2);
}

bool client_writef(Client *client, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = buffer_vappendf(client->output, fmt, args);
    va_end(args);
    return result;
}

bool client_writelnf(Client *client, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = buffer_vappendf(client->output, fmt, args);
    va_end(args);
    if (!result) return false;
    return buffer_append(client->output, "\r\n", 2);
}


bool client_flush(Client *client) {
    if (client->output->length == 0) return true;

    ssize_t bytes = write(client->socket_fd, client->output->data, client->output->length);
    if (bytes <= 0) {
        client->connected = false;
        return false;
    }

    if ((size_t)bytes < client->output->length) {
        memmove(client->output->data,
                client->output->data + bytes,
                client->output->length - bytes);
    }

    client->output->length -= (size_t)bytes;
    client->output->data[client->output->length] = '\0';
    return true;
}


bool client_is_disconnected(const Client *client) {
    return !client->connected;
}

void client_handle_input(Client *client, GameRules *rules, World *world) {
    if (client->input_line_end == 0) {
        return; // No complete line to handle
    }

    if (client->input_handler) {
        client->input_handler(rules, world, client, client->input_buffer);
    }

    // Shift remaining data (if any) left
    size_t consumed = client->input_line_end + 1; // skip over null terminator
    size_t remaining = client->input_length > consumed
        ? client->input_length - consumed
        : 0;

    if (remaining > 0) {
        memmove(client->input_buffer, client->input_buffer + consumed, remaining);
    }

    client->input_length = remaining;
    client->input_line_end = 0;
    client->input_discarding = false;

    // Look for next newline
    for (size_t i = 0; i < client->input_length; ++i) {
        if (client->input_buffer[i] == '\n') {
            client->input_buffer[i] = '\0';
            client->input_line_end = i;
            break;
        }
    }
}
