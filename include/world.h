// world.h
#ifndef WORLD_H
#define WORLD_H

#include <stddef.h>
#include <stdbool.h>
#include "room.h"
#include "actor.h"

typedef struct Player Player;

typedef struct World {
    ActorNode *actors;
    RoomNode  *rooms;
} World;


World *world_new();

void world_add_actor(World *world, Actor *actor);
bool world_remove_actor(World *world, Actor *actor);

void world_add_room(World *world, Room *room);
Room *world_find_room(World *world, RoomID id);

#endif // WORLD_H
