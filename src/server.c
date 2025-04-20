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

#define MAX_CLIENTS 128

static int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Removes trailing newline, carriage return, spaces, and tabs
static void trim_trailing_whitespace(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[--len] = '\0';
    }
}

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

    while (1) {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("poll");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            if (fds[i].revents == 0) continue;

            if (fds[i].fd == listener_fd) {
                // Accept new connection
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

                    // Compact list
                    fds[i] = fds[nfds - 1];
                    clients[i] = clients[nfds - 1];
                    clients[nfds - 1] = NULL;
                    nfds--;
                    i--; // check moved client
                    continue;
                }

                trim_trailing_whitespace(client->input_buffer);
                client_handle_input(client, client->input_buffer);
                client_flush(client);
            }
        }
    }

    close(listener_fd);
    return true;
}

