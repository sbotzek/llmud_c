// actor.c
#include "actor.h"

#include "player.h"
#include "macros.h"
#include "log.h"

static ActorID on_actor_id = 0;

#define MAX_ACTOR_ID UINT32_MAX
#define WARN_ON_ACTOR_ID (UINT32_MAX / 2)

void actor_init(Actor *actor) {
    CHECK(actor != NULL);

    *actor = (Actor){
        .id = ++on_actor_id,
        .alive = false
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

void actor_free(Actor* actor) {
    CHECK(actor != NULL);
    actor_cleanup(actor);
    free(actor);

}
