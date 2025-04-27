#include "telnet_conn.h"
#include "buffer.h"
#include "macros.h"
#include "log.h"
#include "player.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TELNET_CONN_INITIAL_INPUT_CAPACITY 1024
#define TELNET_CONN_INITIAL_OUTPUT_CAPACITY 2048

// Telnet command codes
#define TELNET_IAC   255  // "Interpret As Command"
#define TELNET_DONT  254
#define TELNET_DO    253
#define TELNET_WONT  252
#define TELNET_WILL  251
#define TELNET_SB    250  // Begin subnegotiation
#define TELNET_SE    240  // End subnegotiation

static bool process_input_byte(TelnetConn *conn, unsigned char byte, char *out_char);
static bool write_all(int fd, const char *buf, size_t len);

TelnetConn *telnet_conn_create(int socket_fd, struct sockaddr_in *addr) {
    TelnetConn *conn = malloc(sizeof(TelnetConn));
    if (!conn) {
        return NULL;
    }

    conn->socket_fd = socket_fd;
    conn->address = *addr;
    inet_ntop(AF_INET, &(addr->sin_addr), conn->ip_string, sizeof(conn->ip_string));
    conn->connected = true;

    // Initialize input state
    conn->input_length = 0;
    conn->input_line_ready = false;
    conn->input_discarding = false;

    // Initialize output buffer
    conn->output = buffer_create(TELNET_CONN_INITIAL_OUTPUT_CAPACITY);
    if (!conn->output) {
        free(conn);
        return NULL;
    }

    conn->player = player_create();
    conn->player->conn = conn;

    return conn;
}

void telnet_conn_destroy(TelnetConn *conn) {
    CHECK(conn != NULL);

    if (conn->player) {
        CHECK_MSG(conn->player->conn == conn,
            "Telnet connection %s is linked to player, but conn->player != player",
            conn->ip_string);

        conn->player->conn = NULL;
        conn->player = NULL;
    }

    close(conn->socket_fd);
    buffer_destroy(conn->output);
    free(conn);
}

/*
bool telnet_conn_read(TelnetConn *conn) {
    if (conn->input_line_ready) {
        return true; // Already have a full line
    }

    char temp[TELNET_CONN_MAX_LINE + 1]; // safe size for one full line (+ '\0')
    size_t read_limit;

    if (conn->input_discarding) {
        read_limit = sizeof(temp);
    } else {
        size_t space_left = TELNET_CONN_INPUT_BUFFER_SIZE - conn->input_length;
        read_limit = space_left > sizeof(temp) ? sizeof(temp) : space_left;
        if (read_limit == 0) {
            conn->input_length     = 0;
            conn->input_discarding = true;
            read_limit               = sizeof(temp);
        }
    }

    ssize_t bytes = read(conn->socket_fd, temp, read_limit);
    if (bytes <= 0) {
        conn->connected = false;
        return false;
    }

    for (ssize_t i = 0; i < bytes; ++i) {
        char c;
        if (!process_input_byte(conn, (unsigned char)temp[i], &c)) {
            continue; // not a normal input character
        }

        if (c == '\r') continue;

        if (c == '\n') {
            if (!conn->input_discarding) {
                // Trim trailing
                while (conn->input_length > 0 &&
                       isspace((unsigned char)conn->input_buffer[conn->input_length - 1])) {
                    conn->input_length--;
                }

                // Trim leading
                size_t leading = 0;
                while (leading < conn->input_length &&
                       isspace((unsigned char)conn->input_buffer[leading])) {
                    leading++;
                }

                if (leading > 0 && leading < conn->input_length) {
                    memmove(conn->input_buffer,
                            conn->input_buffer + leading,
                            conn->input_length - leading);
                }

                conn->input_length -= leading;
                conn->input_buffer[conn->input_length] = '\0';

                // **Only change here:**
                conn->input_line_ready = true;
                return true; // One line complete, ready for processing
            } else {
                // Discarded line ends — reset and prepare for next
                conn->input_length     = 0;
                conn->input_discarding = false;
            }
            continue;
        }

        if (conn->input_discarding) {
            continue;
        }

        if (conn->input_length < TELNET_CONN_MAX_LINE) {
            conn->input_buffer[conn->input_length++] = c;
        } else {
            // Line is too long, discard until '\n'
            conn->input_length     = 0;
            conn->input_discarding = true;
        }
    }

    log_trace("telnet_conn_read: ip [%s]: read [%u] bytes, input length [%u]",
              conn->ip_string, bytes, conn->input_length);

    return true;
}
*/

bool telnet_conn_read(TelnetConn *conn) {
    if (conn->input_line_ready) {
        return true; // Already have a full line
    }

    char temp[TELNET_CONN_MAX_LINE + 1];
    size_t read_limit;

    if (conn->input_discarding) {
        read_limit = sizeof(temp);
    } else {
        size_t space_left = TELNET_CONN_INPUT_BUFFER_SIZE - conn->input_length;
        read_limit = space_left > sizeof(temp) ? sizeof(temp) : space_left;
        if (read_limit == 0) {
            conn->input_length     = 0;
            conn->input_discarding = true;
            read_limit             = sizeof(temp);
        }
    }

    ssize_t bytes = read(conn->socket_fd, temp, read_limit);
    if (bytes <= 0) {
        conn->connected = false;
        return false;
    }

    log_trace("telnet_conn_read: ip [%s]: read [%u] bytes", conn->ip_string, bytes);

    for (ssize_t i = 0; i < bytes; ++i) {
        char c;
        if (!process_input_byte(conn, (unsigned char)temp[i], &c)) {
            continue; // not a normal input character
        }

        if (c == '\r') continue;

        if (c == '\n') {
            if (!conn->input_discarding) {
                // Trim trailing
                while (conn->input_length > 0 &&
                       isspace((unsigned char)conn->input_buffer[conn->input_length - 1])) {
                    conn->input_length--;
                }

                // Trim leading
                size_t leading = 0;
                while (leading < conn->input_length &&
                       isspace((unsigned char)conn->input_buffer[leading])) {
                    leading++;
                }

                if (leading > 0 && leading < conn->input_length) {
                    memmove(conn->input_buffer,
                            conn->input_buffer + leading,
                            conn->input_length - leading);
                }
                conn->input_length -= leading;

                // — only change starts here —
                // separate this command with a single '\n' instead of NUL
                conn->input_buffer[conn->input_length] = '\n';
                conn->input_length++;
                // — only change ends here —

                conn->input_line_ready = true;
                return true; // One line complete
            } else {
                // Discarded line ends — reset and prepare for next
                conn->input_length     = 0;
                conn->input_discarding = false;
            }
            continue;
        }

        if (conn->input_discarding) {
            continue;
        }

        if (conn->input_length < TELNET_CONN_MAX_LINE) {
            conn->input_buffer[conn->input_length++] = c;
        } else {
            // Line is too long, discard until '\n'
            conn->input_length     = 0;
            conn->input_discarding = true;
        }
    }

    return true;
}

static bool process_input_byte(TelnetConn *conn, unsigned char byte, char *out_char) {
    switch (conn->telnet_state) {
        case TELNET_STATE_DATA:
            if (byte == TELNET_IAC) {
                conn->telnet_state = TELNET_STATE_IAC;
                return false;
            } else {
                *out_char = byte;
                return true;
            }

        case TELNET_STATE_IAC:
            if (byte == TELNET_IAC) {
                conn->telnet_state = TELNET_STATE_DATA;
                *out_char = TELNET_IAC;
                return true;
            } else if (byte == TELNET_DO || byte == TELNET_DONT ||
                       byte == TELNET_WILL || byte == TELNET_WONT) {
                conn->telnet_command = byte;
                conn->telnet_state = TELNET_STATE_COMMAND;
            } else if (byte == TELNET_SB) {
                conn->telnet_state = TELNET_STATE_SB;
            } else {
                conn->telnet_state = TELNET_STATE_DATA;
            }
            return false;

        case TELNET_STATE_COMMAND:
            log_trace("Telnet command %u %u", conn->telnet_command, byte);
            conn->telnet_state = TELNET_STATE_DATA;
            return false;

        case TELNET_STATE_SB:
            conn->telnet_state = TELNET_STATE_SB_DATA;
            return false;

        case TELNET_STATE_SB_DATA:
            if (byte == TELNET_IAC) {
                conn->telnet_state = TELNET_STATE_SB_IAC;
            }
            return false;

        case TELNET_STATE_SB_IAC:
            if (byte == TELNET_SE) {
                conn->telnet_state = TELNET_STATE_DATA;
            } else {
                conn->telnet_state = TELNET_STATE_SB_DATA;
            }
            return false;
    }

    return false;
}

bool telnet_conn_write(TelnetConn *conn, const char *text) {
    return buffer_append(conn->output, text, strlen(text));
}

bool telnet_conn_writef(TelnetConn *conn, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = buffer_vappendf(conn->output, fmt, args);
    va_end(args);
    return result;
}

bool telnet_conn_vwritef(TelnetConn *conn, const char *fmt, va_list args) {
    return buffer_vappendf(conn->output, fmt, args);
}

bool telnet_conn_flush(TelnetConn *conn) {
    Buffer *b = conn->output;
    if (b->length == 0) return true;

    log_trace("telnet_flush_tick: ip [%s]: flushing [%u] bytes", conn->ip_string, b->length);

    char *data = b->data;
    char *end  = data + b->length;

    // 1) Scan from data→end, flushing segments around lone '\n'
    char *seg_start = data;
    for (char *p = data; p < end; ++p) {
        if (*p == '\n' && (p == data || *(p - 1) != '\r')) {
            // 2a) write the bytes before the '\n'
            if (!write_all(conn->socket_fd, seg_start, p - seg_start) ||
                // 2b) inject CRLF
                !write_all(conn->socket_fd, "\r\n", 2)) {
                conn->connected = false;
                return false;
            }
            seg_start = p + 1;
        }
    }

    // 2) Write any trailing bytes after the last processed '\n'
    if (seg_start < end) {
        if (!write_all(conn->socket_fd, seg_start, end - seg_start)) {
            conn->connected = false;
            return false;
        }
    }

    // 3) All data sent → clear the buffer
    b->length = 0;
    b->data[0] = '\0';
    return true;
}

// Helper: keep writing until all bytes are sent or an error occurs
static bool write_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n <= 0) return false;
        sent += (size_t)n;
    }
    return true;
}

bool telnet_conn_is_disconnected(const TelnetConn *conn) {
    return !conn->connected;
}

bool telnet_conn_next_line(TelnetConn *conn, char *out) {
    if (!conn->input_line_ready) {
        return false;
    }

    // 1. Locate the newline in the buffer
    size_t i = 0;
    while (i < conn->input_length && conn->input_buffer[i] != '\n') {
        ++i;
    }
    // (Shouldn't happen, since input_line_ready is true, but guard anyway)
    if (i >= conn->input_length) {
        conn->input_line_ready = false;
        return false;
    }

    // 2. Determine how many characters to copy (strip trailing '\r')
    size_t copy_len = i;
    if (copy_len > 0 && conn->input_buffer[copy_len - 1] == '\r') {
        --copy_len;
    }
    if (copy_len > TELNET_CONN_MAX_LINE) {
        copy_len = TELNET_CONN_MAX_LINE;
    }

    // 3. Copy into user-supplied buffer and NUL-terminate
    memcpy(out, conn->input_buffer, copy_len);
    out[copy_len] = '\0';

    // 4. Consume that line (skip the '\n') and shift the rest of the data
    size_t consumed = i + 1;
    memmove(conn->input_buffer,
            conn->input_buffer + consumed,
            conn->input_length - consumed);
    conn->input_length -= consumed;

    // 5. Recompute line-ready flag for any remaining full line
    conn->input_line_ready =
        (memchr(conn->input_buffer, '\n', conn->input_length) != NULL);

    return true;
}
