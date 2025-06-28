// helpers.c
#include "helpers.h"
#include "macros.h"
#include "log.h"
#include "appearance.h"
#include "account.h"
#include "strutil.h"
#include <string.h>
#include <stdlib.h>

void helpers_setup(void) {
    // Clear the world
    world_actors = NULL;
}

void helpers_teardown(void) {
    helpers_clear_world();
}

Actor* helpers_create_actor(ActorID id) {
    Actor *actor;
    if (id != INVALID_ACTOR_ID) {
        actor = actor_new_persistent(id);
    } else {
        actor = actor_new();
    }
    return actor;
}

Actor* helpers_create_player_actor(const char *name) {
    CHECK(name != NULL);
    
    Actor *actor = helpers_create_actor(INVALID_ACTOR_ID);
    appearance_init(&actor->appearance);
    actor->appearance.name = str_copy(name);
    str_capitalize(actor->appearance.name);
    
    return actor;
}

Actor* helpers_create_room_actor(ActorID id, const char *name) {
    CHECK(name != NULL);
    
    Actor *actor = helpers_create_actor(id);
    appearance_init(&actor->appearance);
    actor->appearance.name = str_copy(name);
    str_capitalize(actor->appearance.name);
    
    return actor;
}

Player* helpers_create_player(const char *username) {
    CHECK(username != NULL);
    
    Player *player = player_new();
    // Create account with a default password for testing
    player->account = account_new(username, "testpass");
    
    return player;
}

Player* helpers_create_player_with_actor(const char *username, const char *character_name) {
    CHECK(username != NULL);
    CHECK(character_name != NULL);
    
    Player *player = helpers_create_player(username);
    Actor *actor = helpers_create_player_actor(character_name);
    
    // Link player and actor
    player->actor = actor;
    actor->player = player;
    
    return player;
}

void helpers_add_actor_to_world(Actor *actor) {
    CHECK(actor != NULL);
    world_add_actor(actor);
}

void helpers_add_actor_to_location(Actor *actor, Actor *location) {
    CHECK(actor != NULL);
    CHECK(location != NULL);
    
    // First add location to world if not already there
    if (!helpers_actor_exists_in_world(location->id)) {
        helpers_add_actor_to_world(location);
    }
    
    // Then add actor to location
    actor_add_contents(location, actor);
}

bool helpers_actor_exists_in_world(ActorID id) {
    return world_find_actor(id) != NULL;
}

bool helpers_actor_in_location(Actor *actor, Actor *location) {
    CHECK(actor != NULL);
    CHECK(location != NULL);
    
    for (Actor *contents = location->contents; contents; contents = contents->next_contents) {
        if (contents == actor) {
            return true;
        }
    }
    return false;
}

int helpers_count_actors_in_world(void) {
    int count = 0;
    for (Actor *actor = world_actors; actor; actor = actor->next_world) {
        if (!actor->dead) {
            count++;
        }
    }
    return count;
}

int helpers_count_actors_in_location(Actor *location) {
    CHECK(location != NULL);
    
    int count = 0;
    for (Actor *contents = location->contents; contents; contents = contents->next_contents) {
        count++;
    }
    return count;
}

void helpers_free_actor(Actor *actor) {
    CHECK(actor != NULL);
    actor_free(actor);
}

void helpers_free_player(Player *player) {
    CHECK(player != NULL);
    player_free(player);
}

void helpers_clear_world(void) {
    // Remove all actors from world
    Actor *actor = world_actors;
    while (actor) {
        Actor *next = actor->next_world;
        if (!actor->dead) {
            world_remove_actor(actor);
            actor_free(actor);
        }
        actor = next;
    }
    
    // Clear world
    world_actors = NULL;
} 