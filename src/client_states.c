#include "client_states.h"
#include "client.h"
#include "common.h"
#include "world.h"
#include "game_rules.h"

#include <string.h>
#include <stdio.h>

static void handle_menu_input(GameRules *rules, World *world, Client *client, const char *line);
static void handle_playing_input(GameRules *rules, World *world, Client *client, const char *line);
static void handle_character_creation_input(GameRules *rules, World *world, Client *client, const char *line);

void client_state_enter_menu(GameRules *rules, World *world, Client *client) {
    UNUSED(rules);
    UNUSED(world);

    client->state = CLIENT_STATE_MENU;
    client->input_handler = (InputHandler)handle_menu_input;

    client_write(client,
        "\n"
        "Welcome to the game!\n"
        "Type 'play' to enter the game, or 'new' to create a character.\n"
    );
}

void client_state_enter_playing(GameRules *rules, World *world, Client *client) {
    UNUSED(rules);

    client->state = CLIENT_STATE_PLAYING;
    client->input_handler = (InputHandler)handle_playing_input;

    Actor *actor = world_create_actor(world);
    if (!actor) {
        client_write(client, "World is full. Try again later.\n");
        return;
    }

    actor->client = client;
    client->actor = actor;

    client_write(client, "You have entered the world.\n");
}

void client_state_enter_character_creation(GameRules *rules, World *world, Client *client) {
    UNUSED(rules);
    UNUSED(world);

    client->state = CLIENT_STATE_CHARACTER_CREATION;
    client->input_handler = (InputHandler)handle_character_creation_input;

    client_write(client, "Character creation not implemented. Type 'back' to return.\n");
}

static void handle_menu_input(GameRules *rules, World *world, Client *client, const char *line) {
    if (strcmp(line, "play") == 0) {
        client_state_enter_playing(rules, world, client);
    } else if (strcmp(line, "new") == 0) {
        client_state_enter_character_creation(rules, world, client);
    } else {
        client_write(client, "Unknown command. Type 'play' or 'new'.\n");
    }
}

static void handle_playing_input(GameRules *rules, World *world, Client *client, const char *line) {
    if (strcmp(line, "quit") == 0) {
        if (client->actor) {
            world_remove_actor(world, client->actor);
        }

        client_write(client, "You leave the game world.\n");
        client_state_enter_menu(rules, world, client);
        return;
    }

    client_write(client, "You say: ");
    client_write(client, line);
    client_write(client, "\n");
}

static void handle_character_creation_input(GameRules *rules, World *world, Client *client, const char *line) {
    if (strcmp(line, "back") == 0) {
        client_state_enter_menu(rules, world, client);
    } else {
        client_write(client, "Character creation not implemented. Type 'back' to return.\n");
    }
}
