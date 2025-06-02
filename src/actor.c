// actor.c
#include "actor.h"

#include "player.h"
#include "macros.h"
#include "log.h"

static ActorID on_actor_id = MAX_PERSISTENT_ACTOR_ID + 1;

#define MAX_ACTOR_ID UINT32_MAX
#define WARN_ON_ACTOR_ID (UINT32_MAX / 2)

static void actor_init_internal(Actor *actor);

void actor_init_persistent(Actor *actor, ActorID id) {
    CHECK(id <= MAX_PERSISTENT_ACTOR_ID && id != INVALID_ACTOR_ID);
    actor_init_internal(actor);
    actor->id = id;
}


void actor_init(Actor *actor) {
    actor_init_internal(actor);
    actor->id = on_actor_id++;
}

static void actor_init_internal(Actor *actor) {
    CHECK(actor != NULL);

    *actor = (Actor){
        .id = INVALID_ACTOR_ID,
        .location_id = INVALID_ACTOR_ID,
        .dead = true
    };
    appearance_init(&actor->appearance);

    if (on_actor_id > WARN_ON_ACTOR_ID) {
        if (on_actor_id == MAX_ACTOR_ID) {
            log_fatal("Actor ID reached max value: %u", on_actor_id);
        } else {
            log_warn("Actor ID reached warn level: %u", on_actor_id);
        }
    }
}

void actor_cleanup(Actor *actor) {
    CHECK(actor != NULL);

    if (actor->player) {
        CHECK_MSG(actor->player->actor == actor,
                  "actor-player mismatch: actor->id=%u, player->actor->id=%u",
                  actor->id,
                  actor->player->actor ? actor->player->actor->id : INVALID_ACTOR_ID);
        actor->player->actor = NULL;
        actor->player = NULL;
    }

    appearance_cleanup(&actor->appearance);
}

Actor* actor_new() {
    Actor *actor = calloc(1, sizeof(Actor));
    CHECK_MSG(actor != NULL, "actor_new: malloc actor failed");

    actor_init(actor);

    return actor;
}

Actor* actor_new_persistent(ActorID id) {
    Actor *actor = calloc(1, sizeof(Actor));
    CHECK_MSG(actor != NULL, "actor_new: malloc actor failed");

    actor_init_persistent(actor, id);

    return actor;
}

void actor_free(Actor* actor) {
    CHECK(actor != NULL);
    actor_cleanup(actor);
    free(actor);

}
void actor_add_contents(Actor *location, Actor *contents) {
    CHECK(location != NULL);
    CHECK(contents != NULL);
    CHECK(contents->location_id == INVALID_ACTOR_ID);

    contents->next_contents = location->contents;
    location->contents = contents;
    contents->location_id = location->id;
}

void actor_remove_contents(Actor *location, Actor *contents) {
    CHECK(location != NULL);
    CHECK(contents != NULL);
    CHECK(contents->location_id == location->id);

    Actor **pp = &location->contents;
    while (*pp) {
        if (*pp == contents) {
            *pp = (*pp)->next_contents;
            contents->location_id = INVALID_ACTOR_ID;
        }
        pp = &(*pp)->next_contents;
    }
}

void actor_move_contents(Actor *location, Actor *contents, Actor *new_location) {
    CHECK(location      != NULL);
    CHECK(contents      != NULL);
    CHECK(new_location  != NULL);
    CHECK(contents->location_id == location->id);
    CHECK(new_location->id != INVALID_ACTOR_ID);

    if (location->id == new_location->id) {
        return;
    }

    actor_remove_contents(location, contents);
    actor_add_contents(new_location, contents);
}
