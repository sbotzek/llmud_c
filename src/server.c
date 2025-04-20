#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 128
#define BUFFER_SIZE 2048

static int make_socket_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool server_start(int port) {
    int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    make_socket_nonblocking(listener_fd);

    struct pollfd fds[MAX_CLIENTS];
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
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(listener_fd, (struct sockaddr*)&client_addr, &addrlen);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }

                make_socket_nonblocking(client_fd);
                if (nfds < MAX_CLIENTS) {
                    fds[nfds].fd = client_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    printf("New client connected: %s:%d\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                } else {
                    printf("Too many clients! Dropping connection.\n");
                    close(client_fd);
                }

            } else {
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer) - 1);

                if (bytes_read <= 0) {
                    printf("Client disconnected (fd=%d)\n", fds[i].fd);
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--; // recheck the new fds[i]
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Client %d says: %s", fds[i].fd, buffer);
                    write(fds[i].fd, buffer, bytes_read); // echo
                }
            }
        }
    }

    close(listener_fd);
    return true;
}
