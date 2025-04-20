#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <stdbool.h>
#include "buffer.h"

#define CLIENT_NAME_LENGTH 64
#define CLIENT_MAX_LINE    1024  // Max length for a single input line
#define CLIENT_INPUT_BUFFER_SIZE (CLIENT_MAX_LINE * 2) // must be at least 2x CLIENT_MAX_LINE

typedef struct Client Client;

typedef enum {
    CLIENT_STATE_MENU,
    CLIENT_STATE_PLAYING,
    CLIENT_STATE_CHARACTER_CREATION
} ClientState;

typedef void (*InputHandler)(Client *client, const char *line);

struct Client {
    int socket_fd;
    struct sockaddr_in address;
    char ip_string[INET_ADDRSTRLEN];
    bool connected;

    // Input handling
    //
    char input_buffer[CLIENT_INPUT_BUFFER_SIZE]; // Current input buffer
    size_t input_length;                // Number of chars in input_buffer
    size_t input_line_end;             // Index of '\0' that terminates a ready line, or 0 if none
    bool input_discarding;             // True if we're discarding a too-long line

    // Output handling (assumed you're still using a dynamic buffer here)
    Buffer *output;

    // Client state
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

