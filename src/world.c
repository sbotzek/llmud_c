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

    ActorNode *node = calloc(1, sizeof(ActorNode));
    CHECK_MSG(node != NULL, "world_add_actor: calloc ActorNode failed");

    if (location != NULL) {
        actor->location_id = INVALID_ACTOR_ID;
        actor_add_contents(location, actor);
    }

    actor->alive = true;
    node->actor = actor;
    node->next = world->actors;
    world->actors = node;
}

bool world_remove_actor(World *world, Actor *actor) {
    CHECK(world != NULL);
    CHECK(actor != NULL);
    CHECK_MSG(actor->alive,
              "attempted to remove a dead actor (id=%u)", actor->id);

    actor->alive = false;

    ActorNode **pp = &world->actors;
    while (*pp) {
        ActorNode *node = *pp;
        if (node->actor == actor) {
            *pp = node->next;
            node->actor = NULL;
            free(node);
            return true;
        }
    }

    return false;
}

Actor *world_find_actor(World *world, ActorID id) {
    CHECK(world != NULL);
    for (ActorNode *node = world->actors; node; node = node->next) {
        if (node->actor->id == id) return node->actor;
    }
    return NULL;
}
