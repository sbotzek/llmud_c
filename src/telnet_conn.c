// telnet_conn.c
#include "telnet_conn.h"
#include "buffer.h"
#include "macros.h"
#include "log.h"
#include "player.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TELNET_CONN_INITIAL_INPUT_CAPACITY 1024
#define TELNET_CONN_INITIAL_OUTPUT_CAPACITY 2048

// Telnet command codes
#define TELNET_IAC   255
#define TELNET_DONT  254
#define TELNET_DO    253
#define TELNET_WONT  252
#define TELNET_WILL  251
#define TELNET_SB    250
#define TELNET_SE    240

static bool process_input_byte(TelnetConn *conn, unsigned char byte, char *out_char);
static bool write_all(int fd, const char *buf, size_t len);

TelnetConn *telnet_conn_new(int socket_fd, struct sockaddr_in *addr) {
    TelnetConn *conn = calloc(1, sizeof *conn);
    CHECK_MSG(conn, "telnet_conn_new: calloc failed");

    conn->socket_fd = socket_fd;
    conn->address   = *addr;
    inet_ntop(AF_INET, &addr->sin_addr, conn->ip_string, sizeof conn->ip_string);
    conn->connected = true;

    conn->output = dbuffer_new(TELNET_CONN_INITIAL_OUTPUT_CAPACITY);

    return conn;
}

void telnet_conn_free(TelnetConn *conn) {
    CHECK(conn != NULL);

    if (conn->player) {
        CHECK_MSG(conn->player->conn == conn,
            "Telnet connection %s is linked to player, but conn->player != player",
            conn->ip_string);

        conn->player->conn = NULL;
        conn->player = NULL;
    }

    close(conn->socket_fd);
    dbuffer_free(conn->output);
    free(conn);
}

void telnet_conn_disconnect(TelnetConn *conn) {
    CHECK(conn != NULL);
    if (!conn->connected) return;

    close(conn->socket_fd);
    conn->connected = false;
}

void telnet_conn_read(TelnetConn *conn) {
    if (conn->input_line_ready) {
        return;
    }

    char temp[PLAYER_INPUT_SIZE];
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
        return;
    }

    log_trace("telnet_conn_read: ip [%s]: read [%u] bytes", conn->ip_string, bytes);

    for (ssize_t i = 0; i < bytes; ++i) {
        char c;
        if (!process_input_byte(conn, (unsigned char)temp[i], &c)) {
            continue;
        }

        if (c == '\r') continue;

        if (c == '\n') {
            if (!conn->input_discarding) {
                while (conn->input_length > 0 &&
                       isspace((unsigned char)conn->input_buffer[conn->input_length - 1])) {
                    conn->input_length--;
                }

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

                conn->input_buffer[conn->input_length++] = '\n';
                conn->input_line_ready = true;
                return;
            } else {
                conn->input_length     = 0;
                conn->input_discarding = false;
            }
            continue;
        }

        if (conn->input_discarding) {
            continue;
        }

        if (conn->input_length < PLAYER_INPUT_SIZE - 1) {
            conn->input_buffer[conn->input_length++] = c;
        } else {
            conn->input_length     = 0;
            conn->input_discarding = true;
        }
    }
}

void telnet_conn_write(TelnetConn *conn, const char *text) {
    dbuffer_append(conn->output, text, strlen(text));
}

void telnet_conn_writef(TelnetConn *conn, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dbuffer_vappendf(conn->output, fmt, args);
    va_end(args);
}

void telnet_conn_vwritef(TelnetConn *conn, const char *fmt, va_list args) {
    dbuffer_vappendf(conn->output, fmt, args);
}

void telnet_conn_flush(TelnetConn *conn) {
    DynamicBuffer *b = conn->output;
    if (b->length == 0) return;

    log_trace("telnet_flush_tick: ip [%s]: flushing [%u] bytes", conn->ip_string, b->length);

    char *data = b->data;
    char *end  = data + b->length;
    char *seg_start = data;

    for (char *p = data; p < end; ++p) {
        if (*p == '\n' && (p == data || *(p - 1) != '\r')) {
            if (!write_all(conn->socket_fd, seg_start, p - seg_start) ||
                !write_all(conn->socket_fd, "\r\n", 2)) {
                conn->connected = false;
                return;
            }
            seg_start = p + 1;
        }
    }

    if (seg_start < end) {
        if (!write_all(conn->socket_fd, seg_start, end - seg_start)) {
            conn->connected = false;
            return;
        }
    }

    b->length = 0;
    b->data[0] = '\0';
}

static bool write_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, buf + sent, len - sent);
        if (n <= 0) return false;
        sent += (size_t)n;
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
                conn->telnet_state   = TELNET_STATE_COMMAND;
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
    if (copy_len > PLAYER_INPUT_SIZE - 1) {
        copy_len = PLAYER_INPUT_SIZE - 1;
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
