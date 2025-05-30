// world.c
#include "world.h"

#include "macros.h"
#include "log.h"

#include <stdlib.h>

World *world_new() {
    World *world = calloc(1, sizeof(World));
    CHECK_MSG(world != NULL, "world_new: calloc World failed");
    return world;
}

void world_add_actor(World *world, Actor *actor) {
    CHECK(world != NULL);
    CHECK(actor != NULL);

    Actor *location = NULL;
    if (actor->location_id != INVALID_ACTOR_ID) {
        location = world_find_actor(world, actor->location_id);
        CHECK(location != NULL);
    }

    if (location != NULL) {
        actor->location_id = INVALID_ACTOR_ID;
        actor_add_contents(location, actor);
    }

    actor->dead = false;
    actor->next_world = world->actors;
    world->actors = actor;
}

bool world_remove_actor(World *world, Actor *actor) {
    CHECK(world != NULL);
    CHECK(actor != NULL);
    CHECK_MSG(!actor->dead,
              "attempted to remove a dead actor (id=%u)", actor->id);

    actor->dead = true;

    Actor **pp = &world->actors;
    while (*pp) {
        if (*pp == actor) {
            *pp = (*pp)->next_world;
            return true;
        }
    }

    return false;
}

Actor *world_find_actor(World *world, ActorID id) {
    CHECK(world != NULL);
    for (Actor *actor = world->actors; actor; actor = actor->next_world) {
        if (actor->id == id) return actor;
    }
    return NULL;
}
