#include "client_states.h"
#include "client.h"
#include <string.h>

// ----- State Handlers -----

static void handle_menu_input(Client *client, const char *line) {
    if (strcmp(line, "play") == 0) {
        client_state_enter_playing(client);
    } else if (strcmp(line, "create") == 0) {
        client_state_enter_character_creation(client);
    } else {
        client_write(client, "Menu: type 'play' or 'create'\n");
    }
}

static void handle_playing_input(Client *client, const char *line) {
    client_write(client, "You are playing. You typed: ");
    client_write(client, line);
    client_write(client, "\n");
}

static void handle_character_creation_input(Client *client, const char *line) {
    (void)line;

    client_write(client, "Character creation not implemented yet. Returning to menu...\n");
    client_state_enter_menu(client);
}

// ----- State Transitions -----

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
