#include "client.h"
#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

#define CLIENT_INITIAL_INPUT_CAPACITY 1024
#define CLIENT_INITIAL_OUTPUT_CAPACITY 2048

Client *client_create(int socket_fd, struct sockaddr_in *addr) {
    Client *client = malloc(sizeof(Client));
    if (!client) return NULL;

    client->socket_fd = socket_fd;
    client->address = *addr;
    inet_ntop(AF_INET, &(addr->sin_addr), client->ip_string, INET_ADDRSTRLEN);
    client->connected = true;
    client->user_data = NULL;
    client->input_handler = NULL;
    client->state = CLIENT_STATE_MENU;
    client->name[0] = '\0';

    client->input = buffer_create(CLIENT_INITIAL_INPUT_CAPACITY);
    client->output = buffer_create(CLIENT_INITIAL_OUTPUT_CAPACITY);

    if (!client->input || !client->output) {
        client_destroy(client);
        return NULL;
    }

    return client;
}

void client_destroy(Client *client) {
    if (!client) return;

    close(client->socket_fd);

    buffer_destroy(client->input);
    buffer_destroy(client->output);

    free(client);
}

bool client_read(Client *client) {
    char temp[512];
    ssize_t bytes = read(client->socket_fd, temp, sizeof(temp));

    if (bytes <= 0) {
        client->connected = false;
        return false;
    }

    return buffer_append(client->input, temp, (size_t)bytes);
}

bool client_write(Client *client, const char *text) {
    return buffer_append_str(client->output, text);
}

bool client_flush(Client *client) {
    if (client->output->length == 0) return true;

    ssize_t bytes = write(client->socket_fd, client->output->data, client->output->length);
    if (bytes <= 0) {
        client->connected = false;
        return false;
    }

    if ((size_t)bytes < client->output->length) {
        memmove(client->output->data,
                client->output->data + bytes,
                client->output->length - bytes);
    }

    client->output->length -= (size_t)bytes;
    client->output->data[client->output->length] = '\0';
    return true;
}

bool client_is_disconnected(const Client *client) {
    return !client->connected;
}

void client_handle_input(Client *client) {
    if (client->input_handler) {
        client->input_handler(client, client->input);
    } else {
        client_write(client, "No input handler assigned.\n");
    }
}

