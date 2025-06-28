// actor.h
#ifndef ACTOR_H
#define ACTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "appearance.h"
#include "actor_id.h"

typedef struct Player Player;
typedef struct Room Room;
typedef struct Actor Actor;

typedef struct Actor {
    ActorID id;
    bool dead;
    ActorID location_id;
    Actor *contents;
    Appearance appearance;

    Player *player;
    Room *room;

    Actor *next_world;
    Actor *next_contents;
} Actor;

void actor_init(Actor *actor);
void actor_init_persistent(Actor *actor, ActorID id);
void actor_cleanup(Actor *actor);

Actor* actor_new();
Actor* actor_new_persistent(ActorID id);
void actor_free(Actor* actor);

void actor_add_contents(Actor *location, Actor *contents);
void actor_remove_contents(Actor *location, Actor *contents);
void actor_move_contents(Actor *location, Actor *contents, Actor *new_location);

#endif
