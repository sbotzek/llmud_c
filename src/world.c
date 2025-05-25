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

    Room *room = world_find_room(world, actor->in_room_id);
    CHECK(room != NULL);

    ActorNode *node = calloc(1, sizeof(ActorNode));
    CHECK_MSG(node != NULL, "world_add_actor: calloc ActorNode failed");

    actor->in_room_id = INVALID_ROOM_ID;
    room_add_actor(room, actor);

    actor->alive = true;
    node->actor = actor;
    node->next = world->actors;
    world->actors = node;
}

void world_move_actor(World *world, Actor *actor, RoomID new_room_id) {
    CHECK(world != NULL);
    CHECK(actor != NULL);
    CHECK(new_room_id != INVALID_ROOM_ID);

    Room *room = world_find_room(world, actor->in_room_id);
    CHECK(room != NULL);

    Room *new_room = world_find_room(world, new_room_id);
    CHECK(new_room != NULL);

    actor->in_room_id = new_room_id;
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

void world_add_room(World *world, Room *room) {
    CHECK(world != NULL);
    CHECK(room != NULL);

    RoomNode *node = calloc(1, sizeof(RoomNode));
    CHECK_MSG(node != NULL, "world_add_room: calloc RoomNode failed");

    node->room = room;
    node->next = world->rooms;
    world->rooms = node;
}

Room *world_find_room(World *world, RoomID id) {
    CHECK(world != NULL);
    for (RoomNode *node = world->rooms; node; node = node->next) {
        if (node->room->id == id) return node->room;
    }
    return NULL;
}
