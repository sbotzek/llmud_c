// helpers.h
#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include "actor.h"
#include "player.h"
#include "world.h"

// World setup and teardown
void helpers_setup(void);
void helpers_teardown(void);

// Actor creation helpers
Actor* helpers_create_actor(ActorID id);
Actor* helpers_create_player_actor(const char *name);
Actor* helpers_create_room_actor(ActorID id, const char *name);

// Player creation helpers
Player* helpers_create_player(const char *username);
Player* helpers_create_player_with_actor(const char *username, const char *character_name);

// World population helpers
void helpers_add_actor_to_world(Actor *actor);
void helpers_add_actor_to_location(Actor *actor, Actor *location);

// Validation helpers
bool helpers_actor_exists_in_world(ActorID id);
bool helpers_actor_in_location(Actor *actor, Actor *location);
int helpers_count_actors_in_world(void);
int helpers_count_actors_in_location(Actor *location);

// Cleanup helpers
void helpers_free_actor(Actor *actor);
void helpers_free_player(Player *player);
void helpers_clear_world(void);

#endif // HELPERS_H 