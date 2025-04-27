#include "player_states.h"

#include "player.h"
#include "macros.h"
#include "world.h"
#include "log.h"
#include "game_rules.h"

#include <string.h>
#include <stdio.h>

static void handle_menu_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_playing_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_character_creation_input(GameRules *rules, World *world, Player *player, const char *line);

void player_state_enter_menu(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    UNUSED(world);

    player->state = PLAYER_STATE_MENU;
    player->input_handler = (InputHandler)handle_menu_input;

    player_send(player,
        "\n"
        "Welcome to the game!\n"
        "Type 'play' to enter the game, or 'new' to create a character.\n"
    );
}

void player_state_enter_playing(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);

    player->state = PLAYER_STATE_PLAYING;
    player->input_handler = (InputHandler)handle_playing_input;

    Actor *actor = world_create_actor(world);
    if (!actor) {
        player_send(player, "World is full. Try again later.\n");
        return;
    }

    actor->player = player;
    player->actor = actor;

    player_send(player, "You have entered the world.\n");
}

void player_state_enter_character_creation(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    UNUSED(world);

    player->state = PLAYER_STATE_CHARACTER_CREATION;
    player->input_handler = (InputHandler)handle_character_creation_input;

    player_send(player, "Character creation not implemented. Type 'back' to return.\n");
}

static void handle_menu_input(GameRules *rules, World *world, Player *player, const char *line) {
    if (strcmp(line, "play") == 0) {
        player_state_enter_playing(rules, world, player);
    } else if (strcmp(line, "new") == 0) {
        player_state_enter_character_creation(rules, world, player);
    } else {
        player_send(player, "Unknown command. Type 'play' or 'new'.\n");
    }
}

static void handle_playing_input(GameRules *rules, World *world, Player *player, const char *line) {
    if (strcmp(line, "quit") == 0) {
        if (player->actor) {
            world_remove_actor(world, player->actor);
        }

        player_send(player, "You leave the game world.\n");
        player_state_enter_menu(rules, world, player);
        return;
    }

    player_sendf(player, "You say: %s\n", line);
}

static void handle_character_creation_input(GameRules *rules, World *world, Player *player, const char *line) {
    if (strcmp(line, "back") == 0) {
        player_state_enter_menu(rules, world, player);
    } else {
        player_send(player, "Character creation not implemented. Type 'back' to return.\n");
    }
}
