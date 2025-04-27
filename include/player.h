// player.h
#ifndef PLAYER_H
#define PLAYER_H

#include "actor.h"
#include <stddef.h>    // for size_t
#include <stdbool.h>

#define PLAYER_NAME_LENGTH 64

typedef struct GameRules   GameRules;
typedef struct World       World;
typedef struct TelnetConn  TelnetConn;
typedef struct Player      Player;

typedef enum {
    PLAYER_STATE_MENU,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_CHARACTER_CREATION
} PlayerState;

typedef void (*InputHandler)(
    GameRules *game_rules,
    World     *world,
    Player    *player,
    const char *line
);

struct Player {
    char          name[PLAYER_NAME_LENGTH];
    void         *user_data;

    PlayerState   state;
    InputHandler  input_handler;

    Actor        *actor;

    /* link back to the I/O layer */
    TelnetConn   *conn;
};

// — Lifecycle
Player *player_create();
void    player_destroy(Player *player);

// — Input
void    player_handle_input(Player *player, GameRules *rules, World *world, const char *line);

// — Output helpers (wrap telnet_conn_)
void    player_send   (Player *player, const char *text);
void    player_sendf  (Player *player, const char *fmt, ...);

#endif // PLAYER_H
