#include "world.h"

#include "player.h"
#include "macros.h"
#include "log.h"

static bool warned_actor_count = false;

Actor *world_create_actor(World *world) {
    CHECK(world != NULL);

    // Try to reuse a dead slot
    for (size_t i = 0; i < world->actor_count; ++i) {
        if (!world->actors[i].alive) {
            Actor *actor = &world->actors[i];
            *actor = (Actor){
                .id = (ActorID)i,
                .alive = true
                // Other fields zero-initialized
            };
            return actor;
        }
    }

    if (!warned_actor_count &&
        world->actor_count >= (MAX_ACTORS * 9) / 10) {
        warned_actor_count = true;
        log_warn("Actor count nearing capacity: %zu/%d", world->actor_count, MAX_ACTORS);
    }

    CHECK_MSG(world->actor_count < MAX_ACTORS,
              "cannot create actor: actor_count=%zu, max=%d",
              world->actor_count, MAX_ACTORS);

    Actor *actor = &world->actors[world->actor_count];
    *actor = (Actor){
        .id = (ActorID)world->actor_count,
        .alive = true
    };
    world->actor_count++;
    return actor;
}


bool world_remove_actor(World *world, Actor *actor) {
    CHECK(world != NULL);
    CHECK(actor != NULL);
    CHECK_MSG(actor->alive,
              "attempted to remove a dead actor (id=%u)", actor->id);
    CHECK_MSG(actor->id < world->actor_count,
              "actor ID out of bounds: id=%u, actor_count=%zu",
              actor->id, world->actor_count);

    actor->alive = false;

    // Slightly dirty: World knows about player to clear mutual link.
    if (actor->player) {
        CHECK_MSG(actor->player->actor == actor,
                  "actor-player mismatch: actor->id=%u, player->actor->id=%u",
                  actor->id,
                  actor->player->actor ? actor->player->actor->id : INVALID_ACTOR_ID);
        actor->player->actor = NULL;
    }

    return true;
}
