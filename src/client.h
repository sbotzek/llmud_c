#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <stdbool.h>

#define MAX_CLIENT_INPUT 2048
#define MAX_CLIENT_OUTPUT 8192
#define CLIENT_NAME_LENGTH 64

struct Client;
typedef struct Client Client;

// Function pointer type for input handling
typedef void (*InputHandler)(Client *client, const char *input);

typedef enum {
    CLIENT_STATE_MENU,
    CLIENT_STATE_PLAYING,
    CLIENT_STATE_CHARACTER_CREATION
} ClientState;

struct Client {
    int socket_fd;
    struct sockaddr_in address;
    char ip_string[INET_ADDRSTRLEN];
    bool connected;

    char input_buffer[MAX_CLIENT_INPUT];
    int input_length;

    char output_buffer[MAX_CLIENT_OUTPUT];
    int output_length;

    char name[CLIENT_NAME_LENGTH];
    void *user_data;

    ClientState state;
    InputHandler input_handler;
};

Client *client_create(int socket_fd, struct sockaddr_in *addr);
void client_destroy(Client *client);
bool client_read(Client *client);
bool client_write(Client *client, const char *text);
bool client_flush(Client *client);
bool client_is_disconnected(const Client *client);

void client_handle_input(Client *client, const char *input);

#endif // CLIENT_H
