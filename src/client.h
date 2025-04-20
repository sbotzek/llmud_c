#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <stdbool.h>
#include "buffer.h"

#define CLIENT_NAME_LENGTH 64

typedef struct Client Client;

typedef enum {
    CLIENT_STATE_MENU,
    CLIENT_STATE_PLAYING,
    CLIENT_STATE_CHARACTER_CREATION
} ClientState;

// Input handler now takes Buffer*
typedef void (*InputHandler)(Client *client, Buffer *input);

struct Client {
    int socket_fd;
    struct sockaddr_in address;
    char ip_string[INET_ADDRSTRLEN];
    bool connected;

    Buffer *input;
    Buffer *output;

    char name[CLIENT_NAME_LENGTH];
    void *user_data;

    ClientState state;
    InputHandler input_handler;
};

// Lifecycle
Client *client_create(int socket_fd, struct sockaddr_in *addr);
void client_destroy(Client *client);

// I/O
bool client_read(Client *client);
bool client_write(Client *client, const char *text);
bool client_flush(Client *client);
bool client_is_disconnected(const Client *client);

// Input delegation
void client_handle_input(Client *client);

#endif // CLIENT_H

