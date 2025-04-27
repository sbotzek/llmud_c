#ifndef TELNET_CONN_H
#define TELNET_CONN_H

#include "buffer.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>

#define TELNET_CONN_MAX_LINE            1024
#define TELNET_CONN_INPUT_BUFFER_SIZE   (TELNET_CONN_MAX_LINE * 2)

typedef enum {
    TELNET_STATE_DATA,
    TELNET_STATE_IAC,
    TELNET_STATE_COMMAND,
    TELNET_STATE_SB,
    TELNET_STATE_SB_DATA,
    TELNET_STATE_SB_IAC
} TelnetState;

typedef struct World      World;
typedef struct Player     Player;
typedef struct TelnetConn TelnetConn;

// — Lifecycle
TelnetConn *telnet_conn_create(int socket_fd, struct sockaddr_in *addr);
void telnet_conn_destroy(TelnetConn *conn);

// — I/O
bool telnet_conn_read(TelnetConn *conn);

void telnet_conn_write(TelnetConn *conn, const char *text);
void telnet_conn_writef(TelnetConn *conn, const char *fmt, ...);
void telnet_conn_vwritef(TelnetConn *conn, const char *fmt, va_list args);

bool telnet_conn_flush(TelnetConn *conn);
bool telnet_conn_is_disconnected(const TelnetConn *conn);

bool telnet_conn_next_line(TelnetConn *conn, char *out);

struct TelnetConn {
    int socket_fd;
    struct sockaddr_in address;
    char ip_string[INET_ADDRSTRLEN];
    bool connected;

    char input_buffer[TELNET_CONN_INPUT_BUFFER_SIZE];
    size_t input_length;
    bool input_line_ready;
    bool input_discarding;

    Buffer *output;

    TelnetState telnet_state;
    unsigned char telnet_command; // temporarily holds DO/WILL/econn.

    Player *player;
};

#endif // TELNET_CONN_H
