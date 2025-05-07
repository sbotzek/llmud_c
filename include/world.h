/* world.h */
#ifndef WORLD_H
#define WORLD_H

#include <stddef.h>
#include <stdbool.h>
#include "actor.h"

#define MAX_ACTORS 65536

typedef struct Player Player;

typedef struct World {
    Actor    actors[MAX_ACTORS];
    size_t   actor_count;

    /* head of linked list of logged-in players */
    Player  *players_head;
} World;

World *world_new();

/* actor management (unchanged) */
Actor *world_new_actor(World *world);
bool   world_remove_actor(World *world, Actor *actor);

/* player‐list management: */
Player *world_new_player(World *world);
void    world_free_player(World *world, Player *player);
Player *world_find_player_by_username(World *world, const char *username);

#endif // WORLD_H
