// actor.h
#ifndef ACTOR_H
#define ACTOR_H

#include <stdbool.h>
#include "appearance.h"

typedef unsigned int ActorID;
#define INVALID_ACTOR_ID ((ActorID)-1)

typedef struct Player Player;

typedef struct Actor {
    ActorID id;
    bool alive;
    Appearance appearance;
    Player *player;
    // Add other component pointers here
} Actor;

#endif
