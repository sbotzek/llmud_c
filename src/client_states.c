#include "client_states.h"
#include <string.h>
#include <stdio.h>

// Trims whitespace in place (null-terminates as well)
static void trim_trailing_whitespace(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' ||
                       str[len - 1] == ' '  || str[len - 1] == '\t')) {
        str[--len] = '\0';
    }
}

static void handle_menu_input(Client *client, Buffer *input) {
    buffer_append(input, "", 0); // ensure null-terminated
    trim_trailing_whitespace(input->data);

    if (strcmp(input->data, "play") == 0) {
        client_state_enter_playing(client);
    } else if (strcmp(input->data, "create") == 0) {
        client_state_enter_character_creation(client);
    } else {
        client_write(client, "Menu: type 'play' or 'create'\n");
    }

    buffer_clear(input);
}

static void handle_playing_input(Client *client, Buffer *input) {
    buffer_append(input, "", 0);
    trim_trailing_whitespace(input->data);

    buffer_appendf(client->output, "You are playing. You typed: %s\n", input->data);
    buffer_clear(input);
}

static void handle_character_creation_input(Client *client, Buffer *input) {
    client_write(client, "Character creation not implemented yet. Returning to menu...\n");
    buffer_clear(input);
    client_state_enter_menu(client);
}

void client_state_enter_menu(Client *client) {
    client->state = CLIENT_STATE_MENU;
    client->input_handler = handle_menu_input;

    client_write(client, "Welcome to the MUD!\n");
    client_write(client, "Type one of the following:\n");
    client_write(client, "  play   - Enter the game world\n");
    client_write(client, "  create - Begin character creation\n\n");
}

void client_state_enter_playing(Client *client) {
    client->state = CLIENT_STATE_PLAYING;
    client->input_handler = handle_playing_input;

    client_write(client, "You enter the game world...\n");
}

void client_state_enter_character_creation(Client *client) {
    client->state = CLIENT_STATE_CHARACTER_CREATION;
    client->input_handler = handle_character_creation_input;

    client_write(client, "Beginning character creation...\n");
}

