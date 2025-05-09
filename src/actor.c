// actor.c
#include "actor.h"

#include "player.h"
#include "macros.h"

// Initialize an actor with the given ID.
// Other components are zero-initialized. Actor is marked alive.
void actor_init(Actor *actor, ActorID id) {
    CHECK(actor != NULL);

    *actor = (Actor){
        .id = id,
        .alive = false
        // appearance and player default to zero
    };
    appearance_init(&actor->appearance);
}

// Clean up resources owned by the actor. Does not free the actor itself.
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
