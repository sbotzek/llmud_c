// world.h
#ifndef WORLD_H
#define WORLD_H

#include <stddef.h>
#include <stdbool.h>

#include "actor.h"

typedef struct Player Player;

typedef struct ActorNode {
    Actor *actor;
    struct ActorNode *next;
} ActorNode;

typedef struct World {
    ActorNode *actors;
} World;

World *world_new();

/* actor management */
void world_add_actor(World *world, Actor *actor);
bool   world_remove_actor(World *world, Actor *actor);

#endif // WORLD_H
