// world.c
#include "world.h"

#include "macros.h"
#include "log.h"

#include <stdlib.h>

Actor *world_actors = NULL;

void world_add_actor(Actor *actor) {
    CHECK(actor != NULL);

    Actor *location = NULL;
    if (actor->location_id != INVALID_ACTOR_ID) {
        location = world_find_actor(actor->location_id);
        CHECK(location != NULL);
    }

    if (location != NULL) {
        actor->location_id = INVALID_ACTOR_ID;
        actor_add_contents(location, actor);
    }

    actor->dead = false;
    actor->next_world = world_actors;
    world_actors = actor;
}

bool world_remove_actor(Actor *actor) {
    CHECK(actor != NULL);
    CHECK_MSG(!actor->dead,
              "attempted to remove a dead actor (id=%u)", actor->id);

    actor->dead = true;

    Actor **pp = &world_actors;
    while (*pp) {
        if (*pp == actor) {
            *pp = (*pp)->next_world;
            return true;
        }
        pp = &(*pp)->next_world;
    }

    return false;
}

Actor *world_find_actor(ActorID id) {
    for (Actor *actor = world_actors; actor; actor = actor->next_world) {
        if (actor->id == id) return actor;
    }
    return NULL;
}
