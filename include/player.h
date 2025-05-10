// player.h
#ifndef PLAYER_H
#define PLAYER_H

#include <stddef.h>
#include <stdbool.h>

#define PLAYER_NAME_SIZE 16
#define PLAYER_INPUT_SIZE 1024

typedef struct GameRules   GameRules;
typedef struct World       World;
typedef struct TelnetConn  TelnetConn;
typedef struct Player      Player;
typedef struct Actor       Actor;
typedef struct Account     Account;

typedef enum {
    PLAYER_STATE_NONE = 0,
    PLAYER_STATE_MENU,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_CHARACTER_CREATION,
    PLAYER_STATE_ACCOUNT_CREATE,
    PLAYER_STATE_ACCOUNT_LOGIN,
    PLAYER_STATE_ACCOUNT_MENU
} PlayerState;

typedef void (*InputHandler)(
    GameRules *game_rules,
    World     *world,
    Player    *player,
    const char *line
);

struct Player {
    PlayerState   state;
    void         *state_data;
    InputHandler  input_handler;
    Account      *account;
    Actor        *actor;
    TelnetConn   *conn;

    Player       *next_in_registry;
};

// — Lifecycle
Player *player_new(void);
void    player_free(Player *player);

// — Input
void    player_handle_input(Player *player, GameRules *rules, World *world, const char *line);

// — Output
void    player_send(Player *player, const char *text);
void    player_sendf(Player *player, const char *fmt, ...);

void player_create_character(Player *player, const char *name);

char* player_name(Player *player);

void player_register(Player *player);
void player_unregister(Player *player);
Player *player_find_registered(const char *username);

// — Validation
bool player_validate_name(const char *name);

bool player_name_exists(const char *name);

#endif // PLAYER_H
