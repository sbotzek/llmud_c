#include "client.h"

#include "game_process.h"
#include "client_states.h"
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

#define MAX_CLIENTS 128
#define TELNET_PORT 4000

// --- Static process state ---
static int listener_fd = -1;
static Client *clients[MAX_CLIENTS] = {0};

// --- Forward declarations ---
static void accept_new_connections(GameRules *rules, World *world);
static int make_socket_nonblocking(int fd);


void telnet_listen_tick(GameRules *rules, World *world) {
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

    accept_new_connections(rules, world);
}

static void accept_new_connections(GameRules *rules, World *world) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (1) {
        int client_fd = accept(listener_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                log_error("accept() failed: %s", strerror(errno));
            }
            break;
        }

        if (make_socket_nonblocking(client_fd) < 0) {
            log_error("Failed to set client socket non-blocking: %s", strerror(errno));
            close(client_fd);
            continue;
        }

        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (!clients[i]) {
                slot = i;
                break;
            }
        }

        if (slot == -1) {
            log_warn("Too many clients");
            close(client_fd);
            continue;
        }

        clients[slot] = client_create(client_fd, &client_addr);
        if (!clients[slot]) {
            log_error("client_create failed for fd %d", client_fd);
            close(client_fd);
            continue;
        }

        log_info("Client connected: %s", clients[slot]->ip_string);
        client_state_enter_menu(rules, world, clients[slot]);
    }
}

static int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return (flags < 0) ? -1 : fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


void telnet_read_tick(GameRules *rules, World *world) {
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 0;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) continue;

        fds[nfds].fd = clients[i]->socket_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        ++nfds;
    }

    if (poll(fds, nfds, 0) < 0) {
        log_error("poll() failed: %s", strerror(errno));
        return;
    }

    for (int i = 0, slot = 0; i < MAX_CLIENTS; ++i) {
        Client *client = clients[i];
        if (!client) continue;

        if (!(fds[slot].revents & POLLIN)) {
            ++slot;
            continue;
        }

        if (!client_read(client)) {
            log_info("Client disconnected: %s", client->ip_string);
            client_destroy(client);
            clients[i] = NULL;
            ++slot;
            continue;
        }

        client_handle_input(client, rules, world);
        ++slot;
    }
}


void client_flush_tick(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        Client *client = clients[i];
        if (!client) continue;

        client_flush(client);
    }
}
