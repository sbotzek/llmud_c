#include "telnet_conn.h"

#include "game_process.h"
#include "player.h"
#include "player_states.h"
#include "world.h"
#include "log.h"
#include "game_rules.h"
#include "macros.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 128
#define TELNET_PORT 4000

// --- Static process state ---
static int listener_fd = -1;
static TelnetConn *connections[MAX_CONNECTIONS] = {0};

// --- Forward declarations ---
static void accept_new_connections(void);
static int make_socket_nonblocking(int fd);

void telnet_listen_tick(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);
    if (listener_fd < 0) {
        listener_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listener_fd < 0) {
            log_fatal("socket() failed: %s", strerror(errno));
        }

        int opt = 1;
        setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(TELNET_PORT);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(listener_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
            listen(listener_fd, 10) < 0 ||
            make_socket_nonblocking(listener_fd) < 0) {
            log_fatal("Failed to initialize listener: %s", strerror(errno));
        }

        log_info("Listening on port %d...", TELNET_PORT);
    }

    accept_new_connections();
}

static void accept_new_connections() {
    struct sockaddr_in conn_addr;
    socklen_t addrlen = sizeof(conn_addr);

    while (1) {
        int conn_fd = accept(listener_fd, (struct sockaddr *)&conn_addr, &addrlen);
        if (conn_fd < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                log_error("accept() failed: %s", strerror(errno));
            }
            break;
        }

        if (make_socket_nonblocking(conn_fd) < 0) {
            log_error("Failed to set connection socket non-blocking: %s", strerror(errno));
            close(conn_fd);
            continue;
        }

        int slot = -1;
        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            if (!connections[i]) {
                slot = i;
                break;
            }
        }

        if (slot == -1) {
            log_warn("Too many connections");
            close(conn_fd);
            continue;
        }

        connections[slot] = telnet_conn_create(conn_fd, &conn_addr);
        if (!connections[slot]) {
            log_error("telnet_conn_create failed for fd %d", conn_fd);
            close(conn_fd);
            continue;
        }

        player_state_enter_menu(NULL, NULL, connections[slot]->player);
        log_info("New connection: %s", connections[slot]->ip_string);
    }
}

static int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return (flags < 0) ? -1 : fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void telnet_read_tick(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);

    struct pollfd fds[MAX_CONNECTIONS];
    int nfds = 0;

    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if (!connections[i]) continue;

        fds[nfds].fd = connections[i]->socket_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        ++nfds;
    }

    if (poll(fds, nfds, 0) < 0) {
        log_error("poll() failed: %s", strerror(errno));
        return;
    }

    for (int i = 0, slot = 0; i < MAX_CONNECTIONS; ++i) {
        TelnetConn *conn = connections[i];
        if (!conn) continue;

        if (!(fds[slot].revents & POLLIN)) {
            ++slot;
            continue;
        }

        if (!telnet_conn_read(conn)) {
            log_info("disconnected: %s", conn->ip_string);
            telnet_conn_destroy(conn);
            connections[i] = NULL;
            ++slot;
            continue;
        }

        ++slot;
    }
}

void telnet_process_input_tick(GameRules *rules, World *world) {
    char line[TELNET_CONN_MAX_LINE + 1];

    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if (!connections[i]) continue;

        TelnetConn *conn = connections[i];

        if (!telnet_conn_next_line(conn, line)) {
            log_trace("telnet_process_input_tick: ip [%s]: no input", conn->ip_string);
            continue;
        }

        log_debug("telnet_process_input_tick: ip [%s]: found input", conn->ip_string);
        player_handle_input(conn->player, rules, world, line);
    }
}

void telnet_flush_tick(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);

    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        TelnetConn *conn = connections[i];
        if (!conn) continue;

        if (!telnet_conn_flush(conn)) {
            log_info("disconnected: %s", conn->ip_string);
            telnet_conn_destroy(conn);
            connections[i] = NULL;
            continue;
        }

        log_trace("telnet_flush_tick: ip [%s]: flushed", conn->ip_string);
    }
}
