// act.c
#include "act.h"
#include "actor.h"

#include "array.h"
#include "macros.h"
#include "player.h"
#include "world.h"
#include "log.h"

#include <stdlib.h>

static ActListenerFn *act_listener_fns[ACT_MAX];

bool act_run(Act *act) {
    CHECK_MSG(act->type > ACT_NONE && act->type < ACT_MAX, "invalid act type");
    CHECK_MSG(act->actor, "act must have actor");

    for (ActListenerFn *fn = act_listener_fns[act->type]; fn && *fn; fn++) {
        switch ((*fn)(ACT_PHASE_PERFORM, act)) {
            case ACT_LISTENER_CONTINUE: break;
            case ACT_LISTENER_CANCEL: return false;
            case ACT_LISTENER_ACCEPT: return true;
        }

        if (act->actor->dead) {
            return false;
        }
    }

    bool success = act->perform_fn(act);

    for (ActListenerFn *fn = act_listener_fns[act->type]; fn && *fn; fn++) {
        (*fn)(ACT_PHASE_RESOLVE, act);
        if (act->actor->dead) {
            return success;
        }
    }

    return success;
}

void act_register_listener(ActType type, ActListenerFn fn) {
    CHECK_MSG(type > ACT_NONE && type < ACT_MAX, "invalid act type");
    CHECK(fn != NULL);

    if (!act_listener_fns[type]) {
        act_listener_fns[type] = calloc(2, sizeof(fn));
        CHECK_MSG(act_listener_fns[type] != NULL, "calloc failed");
        act_listener_fns[type][0] = fn;
        act_listener_fns[type][1] = NULL;
        return;
    }

    size_t count = 0;
    while (act_listener_fns[type][count]) {
        ++count;
    }

    act_listener_fns[type] = realloc(act_listener_fns[type], (count + 2) * sizeof(fn));
    CHECK_MSG(act_listener_fns[type] != NULL, "realloc failed");

    act_listener_fns[type][count] = fn;
    act_listener_fns[type][count + 1] = NULL;
}

void act_perceive_to(Act *act, Actor *viewer) {
    ActPerceiveFn fn = NULL;

    if (!viewer->player) {
        fn = act->ai_perceive_fn;
    } else if (viewer->player) {
        fn = act->player_perceive_fn;
    }

    if (fn) {
        (*fn)(act, viewer);
    }
}

void act_perceive_at(Act *act) {
    act_perceive_location_except(act, act->actor->location_id, NULL);
}

void act_perceive_at_except(Act *act, Actor *exclude[]) {
    act_perceive_location_except(act, act->actor->location_id, exclude);
}

void act_perceive_location(Act *act, ActorID location_id) {
    act_perceive_location_except(act, location_id, NULL);
}

void act_perceive_location_except(Act *act, ActorID location_id, Actor *exclude[]) {
    log_trace("type %d, actor %u, location %u", act->type, act->actor->id, location_id);
    Actor *location;

    if (location_id != INVALID_ACTOR_ID) {
        location = world_find_actor(act->actor->location_id);
        if (!location) {
            log_error("unable to find location [%u] for actor [%u]", act->actor->id, location_id);
        }
    } else {
        location = NULL;
    }

    if (location) {
        log_trace("found location %u", location->id);
        if (!array_contains(exclude, location)) {
            log_trace("percieving to location %u", location->id);
            act_perceive_to(act, location);
        }

        for (Actor *observer = location->contents; observer; observer = observer->next_contents) {
            log_trace("found observer %u", observer->id);
            if (observer->dead) continue;

            if (!array_contains(exclude, observer)) {
                log_trace("percieving to observer %u", observer->id);
                act_perceive_to(act, observer);
            }
        }
    }
}
