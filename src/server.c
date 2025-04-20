#define _POSIX_C_SOURCE 200112L
#include "server.h"
#include "client.h"
#include "client_states.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h> // for clock_gettime and nanosleep

#define MAX_CLIENTS 128
#define SERVER_TICK_MS 250

static int make_socket_nonblocking(int fd);
static long ms_diff(struct timespec a, struct timespec b);

bool server_start(int port) {
    int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listener_fd);
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listener_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(listener_fd);
        return false;
    }

    if (listen(listener_fd, 10) < 0) {
        perror("listen");
        close(listener_fd);
        return false;
    }

    if (make_socket_nonblocking(listener_fd) < 0) {
        perror("make_socket_nonblocking");
        close(listener_fd);
        return false;
    }

    struct pollfd fds[MAX_CLIENTS];
    Client* clients[MAX_CLIENTS] = {0};
    int nfds = 1;

    fds[0].fd = listener_fd;
    fds[0].events = POLLIN;

    printf("Server listening on port %d...\n", port);

    struct timespec tick_start, tick_end;

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &tick_start);

        int poll_count = poll(fds, nfds, 0);
        if (poll_count < 0) {
            perror("poll");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            if (fds[i].revents == 0) continue;

            if (fds[i].fd == listener_fd) {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(listener_fd, (struct sockaddr*)&client_addr, &addrlen);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                if (make_socket_nonblocking(client_fd) < 0) {
                    perror("make_socket_nonblocking (client)");
                    close(client_fd);
                    continue;
                }

                if (nfds >= MAX_CLIENTS) {
                    printf("Too many clients. Dropping connection.\n");
                    close(client_fd);
                    continue;
                }

                Client *client = client_create(client_fd, &client_addr);
                if (!client) {
                    perror("client_create");
                    close(client_fd);
                    continue;
                }

                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                clients[nfds] = client;
                nfds++;

                printf("New client connected from %s\n", client->ip_string);
                client_state_enter_menu(client);
                client_flush(client);
            } else {
                Client *client = clients[i];
                if (!client) continue;

                if (!client_read(client)) {
                    printf("Client disconnected: %s\n", client->ip_string);
                    client_destroy(client);
                    close(fds[i].fd);

                    fds[i] = fds[nfds - 1];
                    clients[i] = clients[nfds - 1];
                    clients[nfds - 1] = NULL;
                    nfds--;
                    i--;
                    continue;
                }

                client_handle_input(client);
                client_flush(client);
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &tick_end);
        long elapsed_ms = ms_diff(tick_end, tick_start);

        if (elapsed_ms > SERVER_TICK_MS) {
            fprintf(stderr, "[WARN] Server tick took %ldms (limit is %dms)\n", elapsed_ms, SERVER_TICK_MS);
        } else {
            long remaining = SERVER_TICK_MS - elapsed_ms;
            struct timespec delay;
            delay.tv_sec = remaining / 1000;
            delay.tv_nsec = (remaining % 1000) * 1000000L;
            nanosleep(&delay, NULL);
        }
    }

    close(listener_fd);
    return true;
}

static int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static long ms_diff(struct timespec a, struct timespec b) {
    return (a.tv_sec - b.tv_sec) * 1000L +
           (a.tv_nsec - b.tv_nsec) / 1000000L;
}
