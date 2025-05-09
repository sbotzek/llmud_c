// player_states.c
#include "player_states.h"
#include "player.h"
#include "telnet_conn.h"
#include "macros.h"
#include "world.h"
#include "log.h"
#include "game_rules.h"
#include "account.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/* Context for account creation */
typedef enum {
    ACC_CREATE_USERNAME,
    ACC_CREATE_PASSWORD,
    ACC_CREATE_CONFIRM
} AccountCreateStep;

typedef struct {
    AccountCreateStep step;
    char    username[ACCOUNT_USERNAME_SIZE];
    char   *password;
} AccountCreateContext;

/* Context for account login */
typedef enum {
    ACC_LOGIN_USERNAME,
    ACC_LOGIN_PASSWORD
} AccountLoginStep;

typedef struct {
    AccountLoginStep step;
    char             username[ACCOUNT_USERNAME_SIZE];
    Account         *account;
} AccountLoginContext;

/* Check if a username is already in use by any logged-in player */
static bool account_username_exists(World *world, const char *username);

/* Input handlers */
static void handle_menu_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_account_create_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_account_login_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_account_menu_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_playing_input(GameRules *rules, World *world, Player *player, const char *line);
static void handle_character_creation_input(GameRules *rules, World *world, Player *player, const char *line);

/* — Public state entry functions */

void player_state_enter_menu(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    UNUSED(world);
    player->state = PLAYER_STATE_MENU;
    player->input_handler = handle_menu_input;
    player_send(player,
        "\n"
        "Welcome to the game!\n"
        "Type 'create' to create a new account, or 'login' to log in.\n"
    );
}

void player_state_enter_account_create(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    UNUSED(world);
    player->state = PLAYER_STATE_ACCOUNT_CREATE;
    player->input_handler = handle_account_create_input;
    AccountCreateContext *ctx = malloc(sizeof *ctx);
    CHECK_MSG(ctx, "Out of memory");
    ctx->step = ACC_CREATE_USERNAME;
    ctx->password = NULL;
    player->state_data = ctx;
    player_send(player, "Enter username: ");
}

void player_state_enter_account_login(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    UNUSED(world);
    player->state = PLAYER_STATE_ACCOUNT_LOGIN;
    player->input_handler = handle_account_login_input;
    AccountLoginContext *ctx = malloc(sizeof *ctx);
    CHECK_MSG(ctx, "Out of memory");
    ctx->step = ACC_LOGIN_USERNAME;
    ctx->account = NULL;
    player->state_data = ctx;
    player_send(player, "Enter username: ");
}

void player_state_enter_account_menu(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    UNUSED(world);
    player->state = PLAYER_STATE_ACCOUNT_MENU;
    player->input_handler = handle_account_menu_input;
    player_send(player,
        "Account menu:\n"
        "Type 'create' to create a character, 'play <name>' to enter the game, or 'quit' to disconnect.\n"
    );
}

void player_state_enter_playing(GameRules *rules, World *world, Player *player) {
    UNUSED(rules);
    player->state = PLAYER_STATE_PLAYING;
    player->input_handler = handle_playing_input;
    Actor *actor = world_new_actor(world);
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
    player->input_handler = handle_character_creation_input;
    player_send(player, "Enter character name (a-z only): ");
}

/* — Static helper */
static bool account_username_exists(World *world, const char *username) {
    UNUSED(world);  /* no longer used */
    Account *acct = account_load(username);
    if (!acct) {
        return false;
    }
    account_free(acct);
    return true;
}

/* — Static input handlers */

static void handle_menu_input(GameRules *rules, World *world, Player *player, const char *line) {
    CHECK(rules != NULL);
    CHECK(world != NULL);
    CHECK(player != NULL);

    if (strcmp(line, "create") == 0) {
        player_state_enter_account_create(rules, world, player);
    } else if (strcmp(line, "login") == 0) {
        player_state_enter_account_login(rules, world, player);
    } else {
        player_send(player, "Unknown command. Type 'create' or 'login'.\n");
    }
}

static void handle_account_create_input(GameRules *rules, World *world, Player *player, const char *line) {
    CHECK(rules != NULL);
    CHECK(world != NULL);
    CHECK(player != NULL);
    AccountCreateContext *ctx = player->state_data;
    switch (ctx->step) {
    case ACC_CREATE_USERNAME:
        if (!account_validate_username(line)) {
            player_send(player, "Invalid username. Enter username: ");
        } else if (account_username_exists(world, line)) {
            player_send(player, "Username already in use. Enter username: ");
        } else {
            strncpy(ctx->username, line, ACCOUNT_USERNAME_SIZE);
            ctx->username[ACCOUNT_USERNAME_SIZE-1] = '\0';
            ctx->step = ACC_CREATE_PASSWORD;
            player_send(player, "Enter password: ");
        }
        break;
    case ACC_CREATE_PASSWORD:
        if (line[0] == '\0') {
            player_send(player, "Password cannot be empty. Enter password: ");
        } else {
            size_t len = strlen(line);
            ctx->password = malloc(len + 1);
            CHECK_MSG(ctx->password, "Out of memory");
            memcpy(ctx->password, line, len + 1);
            ctx->step = ACC_CREATE_CONFIRM;
            player_send(player, "Confirm password: ");
        }
        break;
    case ACC_CREATE_CONFIRM:
        if (strcmp(line, ctx->password) != 0) {
            free(ctx->password);
            ctx->password = NULL;
            ctx->step = ACC_CREATE_PASSWORD;
            player_send(player, "Passwords do not match. Enter password: ");
        } else {
            Account *acct = account_new(ctx->username, ctx->password);
            account_save(acct);
            free(ctx->password);
            free(ctx);
            player->state_data = NULL;
            player->account = acct;
            player_send(player, "Account created.\n");
            player_state_enter_account_menu(rules, world, player);
        }
        break;
    }
}

static void handle_account_login_input(GameRules *rules, World *world, Player *player, const char *line) {
    CHECK(rules != NULL);
    CHECK(world != NULL);
    CHECK(player != NULL);
    AccountLoginContext *ctx = player->state_data;
    switch (ctx->step) {
    case ACC_LOGIN_USERNAME:
        if (!account_validate_username(line)) {
            player_send(player, "Invalid username. Enter username: ");
        } else {
            Account *loaded = account_load(line);
            if (!loaded) {
                player_send(player, "No such account. Enter username: ");
            } else {
                strncpy(ctx->username, line, ACCOUNT_USERNAME_SIZE);
                ctx->username[ACCOUNT_USERNAME_SIZE-1] = '\0';
                ctx->account = loaded;
                ctx->step    = ACC_LOGIN_PASSWORD;
                player_send(player, "Enter password: ");
            }
        }
        break;

    case ACC_LOGIN_PASSWORD:
        if (ctx->account && account_check_password(ctx->account, line)) {
            Player *existing = player_find_registered(ctx->username);
            if (existing) {
                /* takeover existing Player */
                if (existing->conn) {
                    existing->conn->player = NULL;
                    existing->conn->connected = false;
                }

                player->conn->player = existing;
                existing->conn = player->conn;
                player->conn = NULL;

                /* clean up just the temp Account and temp Player */
                account_free(ctx->account);
                free(ctx);
                player_unregister(player);
                player_free(player);

                player_send(existing, "Login successful.\n");
                player_state_enter_account_menu(rules, world, existing);
            } else {
                /* fresh login: just bind account — registry auto-maintained */
                player->state_data = NULL;
                player->account    = ctx->account;
                free(ctx);

                player_send(player, "Login successful.\n");
                player_state_enter_account_menu(rules, world, player);
            }
        } else {
            player_send(player, "Invalid password. Enter password: ");
        }
        break;
    }
}

static void handle_account_menu_input(GameRules *rules, World *world, Player *player, const char *line) {
    CHECK(rules != NULL);
    CHECK(world != NULL);
    CHECK(player != NULL);
    if (strcmp(line, "quit") == 0) {
        if (player->actor) {
            world_remove_actor(world, player->actor);
        }
        player_send(player, "Goodbye!\n");
        player->conn->player = NULL;
        player->conn->connected = false;
        player_unregister(player);
        player_free(player);
    } else if (strcmp(line, "create") == 0) {
        player_state_enter_character_creation(rules, world, player);
    } else if (strncmp(line, "play", 4) == 0) {
        char charname[PLAYER_NAME_SIZE];
        int got = sscanf(line, "play %63s", charname);
        if (got != 1) {
            player_send(player, "Available characters:\n");
            player_send(player, "  (no characters)\n");
        } else {
            bool found = false;
            if (!found) {
                player_send(player, "Unknown character. Available characters:\n");
                player_send(player, "  (no characters)\n");
            } else {
                snprintf(player->name, PLAYER_NAME_SIZE, "%s", charname);
                player_state_enter_playing(rules, world, player);
            }
        }
    } else {
        player_send(player, "Unknown command. Type 'create', 'play <name>', or 'quit'.\n");
    }
}

static void handle_playing_input(GameRules *rules, World *world, Player *player, const char *line) {
    CHECK(rules != NULL);
    CHECK(world != NULL);
    CHECK(player != NULL);
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
    CHECK(rules != NULL);
    CHECK(world != NULL);
    CHECK(player != NULL);
    size_t len = strlen(line);
    bool valid = len > 0 && len < PLAYER_NAME_SIZE - 1;
    for (size_t i = 0; valid && i < len; ++i) {
        if (line[i] < 'a' || line[i] > 'z') {
            valid = false;
        }
    }
    if (!valid) {
        player_send(player, "Invalid name. Use letters a-z only. Enter character name: ");
    } else {
        snprintf(player->name, PLAYER_NAME_SIZE, "%s", line);
        player_sendf(player, "Character '%s' created.\n", line);
        player_state_enter_account_menu(rules, world, player);
    }
}
