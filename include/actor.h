// actor.h
#ifndef ACTOR_H
#define ACTOR_H

#include <stdbool.h>

typedef unsigned int ActorID;
#define INVALID_ACTOR_ID ((ActorID)-1)

typedef struct Player Player;

typedef struct Actor {
    ActorID id;
    bool alive;
    Player *player;
    // Add other component pointers here
} Actor;

#endif
