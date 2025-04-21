#ifndef ACTOR_H
#define ACTOR_H

#include <stdbool.h>

typedef unsigned int ActorID;
#define INVALID_ACTOR_ID ((ActorID)-1)

typedef struct Client Client;

typedef struct Actor {
    ActorID id;
    bool alive;
    Client *client;
    // Add other component pointers here
} Actor;

#endif
