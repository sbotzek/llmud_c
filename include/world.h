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
} World;

World *world_new();

/* actor management (unchanged) */
Actor *world_new_actor(World *world);
bool   world_remove_actor(World *world, Actor *actor);

#endif // WORLD_H
