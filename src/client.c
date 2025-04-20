#include "client.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

Client *client_create(int socket_fd, struct sockaddr_in *addr) {
    Client *client = calloc(1, sizeof(Client));
    if (!client) return NULL;

    client->socket_fd = socket_fd;
    client->address = *addr;
    inet_ntop(AF_INET, &(addr->sin_addr), client->ip_string, INET_ADDRSTRLEN);
    client->connected = true;
    return client;
}

void client_destroy(Client *client) {
    if (!client) return;
    close(client->socket_fd);
    free(client);
}

bool client_read(Client *client) {
    ssize_t bytes = read(client->socket_fd, client->input_buffer, MAX_CLIENT_INPUT - 1);
    if (bytes <= 0) {
        client->connected = false;
        return false;
    }
    client->input_buffer[bytes] = '\0';
    char *newline = strchr(client->input_buffer, '\n');
    if (newline) *newline = '\0';
    return true;
}

bool client_write(Client *client, const char *text) {
    size_t len = strlen(text);
    if (len + client->output_length >= MAX_CLIENT_OUTPUT) return false;
    memcpy(client->output_buffer + client->output_length, text, len);
    client->output_length += len;
    return true;
}

bool client_flush(Client *client) {
    if (client->output_length == 0) return true;
    ssize_t bytes = write(client->socket_fd, client->output_buffer, client->output_length);
    if (bytes <= 0) {
        client->connected = false;
        return false;
    }
    if (bytes < client->output_length) {
        memmove(client->output_buffer, client->output_buffer + bytes, client->output_length - bytes);
    }
    client->output_length -= bytes;
    return true;
}

bool client_is_disconnected(const Client *client) {
    return !client->connected;
}

void client_handle_input(Client *client, const char *input) {
    if (client->input_handler) {
        client->input_handler(client, input);
    } else {
        client_write(client, "No input handler assigned.\n");
    }
}

