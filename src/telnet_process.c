#include "game_process.h"
#include "client_states.h"
#include "world.h"
#include "game_rules.h"
#include "client.h"

#include <stdio.h>
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
static int make_socket_nonblocking(int fd);
static void accept_new_connections(GameRules *rules, World *world);
static void telnet_tick(GameRules *rules, World *world);

// --- GameProcess factory ---
GameProcess telnet_process(void) {
    return (GameProcess){
        .name = "telnet",
        .tick = telnet_tick,
        .frequency = 1
    };
}

// --- Main process tick ---
static void telnet_tick(GameRules *rules, World *world) {
    if (listener_fd < 0) {
        // Create and bind listener socket
        listener_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listener_fd < 0) {
            perror("socket");
            return;
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
            perror("listener setup");
            close(listener_fd);
            listener_fd = -1;
            return;
        }

        printf("[telnet] Listening on port %d...\n", TELNET_PORT);
    }

    accept_new_connections(rules, world);

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
        perror("poll");
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
            printf("[telnet] Client disconnected: %s\n", client->ip_string);
            client_destroy(client);
            clients[i] = NULL;
            ++slot;
            continue;
        }

        client_handle_input(client, rules, world);
        client_flush(client);
        ++slot;
    }

}

// --- Helper: accept new clients ---
static void accept_new_connections(GameRules *rules, World *world) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (1) {
        int client_fd = accept(listener_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("accept");
            }
            break;
        }

        if (make_socket_nonblocking(client_fd) < 0) {
            perror("make_socket_nonblocking (client)");
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
            printf("[telnet] Too many clients\n");
            close(client_fd);
            continue;
        }

        clients[slot] = client_create(client_fd, &client_addr);
        if (!clients[slot]) {
            perror("client_create");
            close(client_fd);
            continue;
        }

        printf("[telnet] Client connected: %s\n", clients[slot]->ip_string);
        client_state_enter_menu(rules, world, clients[slot]);
        client_flush(clients[slot]);
    }
}

// --- Helper: make socket non-blocking ---
static int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return (flags < 0) ? -1 : fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
