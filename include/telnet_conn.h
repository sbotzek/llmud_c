// telnet_conn.h
#ifndef TELNET_CONN_H
#define TELNET_CONN_H

#include "buffer.h"
#include "player.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

// store two lines + null terminator – we only really want 1 line at a time,
// but we need more storage in case we read part of another line while still
// trying to finish reading an overly long first line.
#define TELNET_CONN_INPUT_BUFFER_SIZE   (2 * (PLAYER_INPUT_SIZE) + 1)

typedef enum {
    TELNET_STATE_DATA,
    TELNET_STATE_IAC,
    TELNET_STATE_COMMAND,
    TELNET_STATE_SB,
    TELNET_STATE_SB_DATA,
    TELNET_STATE_SB_IAC
} TelnetState;

typedef struct Player     Player;
typedef struct TelnetConn TelnetConn;

// — Lifecycle
TelnetConn *telnet_conn_new(int socket_fd, struct sockaddr_in *addr);
void        telnet_conn_free(TelnetConn *conn);

// — I/O
void telnet_conn_read(TelnetConn *conn);

void telnet_conn_write(TelnetConn *conn, const char *text);
void telnet_conn_writef(TelnetConn *conn, const char *fmt, ...);
void telnet_conn_vwritef(TelnetConn *conn, const char *fmt, va_list args);

void telnet_conn_flush(TelnetConn *conn);

// — Line handling
bool telnet_conn_next_line(TelnetConn *conn, char *out);

struct TelnetConn {
    int socket_fd;
    struct sockaddr_in address;
    char ip_string[INET_ADDRSTRLEN];
    bool connected;

    char   input_buffer[TELNET_CONN_INPUT_BUFFER_SIZE];
    size_t input_length;
    bool   input_line_ready;
    bool   input_discarding;

    Buffer *output;

    TelnetState    telnet_state;
    unsigned char  telnet_command;

    Player *player;
};

#endif // TELNET_CONN_H
