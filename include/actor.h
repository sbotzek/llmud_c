// actor.h
#ifndef ACTOR_H
#define ACTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "appearance.h"
#include "room.h"

typedef uint32_t ActorID;
#define INVALID_ACTOR_ID 0

typedef struct Player Player;

typedef struct Actor {
    ActorID id;
    bool alive;
    Player *player;

    Appearance appearance;

    RoomID in_room_id;

} Actor;

typedef struct ActorNode {
    Actor *actor;
    struct ActorNode *next;
} ActorNode;

void actor_init(Actor *actor);
void actor_cleanup(Actor *actor);

Actor* actor_new();
void actor_free(Actor* actor);

#endif
