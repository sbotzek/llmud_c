// world.h
#ifndef WORLD_H
#define WORLD_H

#include <stddef.h>
#include <stdbool.h>
#include "room.h"
#include "actor.h"

typedef struct Player Player;

typedef struct World {
    Actor *actors;
} World;


World *world_new();

void world_add_actor(World *world, Actor *actor);
bool world_remove_actor(World *world, Actor *actor);
Actor *world_find_actor(World *world, ActorID id);

#endif // WORLD_H
