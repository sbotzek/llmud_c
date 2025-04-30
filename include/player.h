/* player.h */
#ifndef PLAYER_H
#define PLAYER_H

#include <stddef.h>
#include <stdbool.h>

#define PLAYER_NAME_LENGTH 64

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
    char          name[PLAYER_NAME_LENGTH];
    PlayerState   state;
    void         *state_data;
    InputHandler  input_handler;
    Account      *account;
    Actor        *actor;
    TelnetConn   *conn;

    /* linked‐list pointer for registry of all players */
    Player       *next_in_registry;
};

// — Lifecycle
Player *player_create(void);
void    player_destroy(Player *player);

// — Input
void    player_handle_input(Player *player, GameRules *rules, World *world, const char *line);

// — Output
void    player_send(Player *player, const char *text);
void    player_sendf(Player *player, const char *fmt, ...);

#endif // PLAYER_H
