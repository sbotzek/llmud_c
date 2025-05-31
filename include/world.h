// world.h
#ifndef WORLD_H
#define WORLD_H

#include <stddef.h>
#include <stdbool.h>
#include "actor.h"

extern Actor *world_actors;

void world_add_actor(Actor *actor);
bool world_remove_actor(Actor *actor);
Actor *world_find_actor(ActorID id);

#endif // WORLD_H
