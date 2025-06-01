// act.c
#include "act.h"

#include <stdlib.h>
#include "world.h"
#include "actor.h"
#include "player.h"
#include "macros.h"

typedef struct ActData {
    ActPrepareFn prepare_fn;
    ActCommitFn commit_fn;
    ActPerceiveFn ai_perceive_fn;;
    ActPerceiveFn player_perceive_fn;;

    ActListenerFn *listener_fns;
} ActData;

static ActData act_data_list[MAX_ACT];

static bool act_null_prepare(Actor *actor, void *act);
static void act_null_commit(Actor *actor, void *act);
static void act_null_perceive(Actor *viewer, Actor *actor, void *act);

static void act_perceive(const ActData *act_data, Actor *actor, void *act);
static void actor_perceive(Actor *viewer, const ActData *act_data, Actor *actor, void *act);

void act_execute(ActType type, Actor *actor, void *act) {
    ActData *act_data = &act_data_list[type];

    if (!act_data->prepare_fn(actor, act) || actor->dead) {
        actor_perceive(actor, act_data, actor, act);
        return;
    }

    for (ActListenerFn *fn = act_data->listener_fns; *fn != NULL; fn++) {
        if (!(*fn)(ACT_PHASE_PREPARE, type, actor, act) || actor->dead) {
            return;
        }
    }

    act_data->commit_fn(actor, act);
    for (ActListenerFn *fn = act_data->listener_fns; *fn != NULL; fn++) {
        (*fn)(ACT_PHASE_COMMIT, type, actor, act);
        if (actor->dead) {
            return;
        }
    }

    act_perceive(act_data, actor, act);
    for (ActListenerFn *fn = act_data->listener_fns; *fn != NULL; fn++) {
        (*fn)(ACT_PHASE_PERCEIVE, type, actor, act);
        if (actor->dead) {
            return;
        }
    }
}

void act_configure(ActType type, ActPrepareFn prepare_fn, ActCommitFn commit_fn, ActPerceiveFn player_fn, ActPerceiveFn ai_fn) {
    CHECK_MSG(type > ACT_NONE && type < MAX_ACT, "Invalid actType");

    ActData *act_data = &act_data_list[type];

    if (!prepare_fn) prepare_fn = act_null_prepare;
    if (!commit_fn) commit_fn = act_null_commit;
    if (!player_fn) player_fn = act_null_perceive;
    if (!ai_fn) ai_fn = act_null_perceive;

    act_data->prepare_fn = prepare_fn;
    act_data->commit_fn = commit_fn;
    act_data->player_perceive_fn = player_fn;
    act_data->ai_perceive_fn = ai_fn;
    act_data->listener_fns = calloc(1, sizeof(*act_data->listener_fns));
}

void act_register_listener(ActType type, ActListenerFn fn) {
    CHECK_MSG(type > ACT_NONE && type < MAX_ACT, "Invalid actType");
    CHECK(fn != NULL);

    ActData *act_data = &act_data_list[type];

    if (!act_data->listener_fns) {
        act_data->listener_fns = calloc(2, sizeof(fn));
        CHECK_MSG(act_data->listener_fns != NULL, "act_register_listener: calloc failed");
        act_data->listener_fns[0] = fn;
        act_data->listener_fns[1] = NULL;
        return;
    }

    size_t count = 0;
    while (act_data->listener_fns[count]) {
        ++count;
    }

    act_data->listener_fns = realloc(act_data->listener_fns, (count + 2) * sizeof(fn));
    CHECK_MSG(act_data->listener_fns != NULL, "act_register_listener: realloc failed");
    act_data->listener_fns[count] = fn;
    act_data->listener_fns[count + 1] = NULL;
}

static bool act_null_prepare(Actor *actor, void *act) {
    UNUSED(actor);
    UNUSED(act);

    return true;
}

static void act_null_commit(Actor *actor, void *act) {
    UNUSED(actor);
    UNUSED(act);
}

static void act_null_perceive(Actor *viewer, Actor *actor, void *act) {
    UNUSED(viewer);
    UNUSED(actor);
    UNUSED(act);
}

static void act_perceive(const ActData *act_data, Actor *actor, void *act) {
    if (!actor->dead) {
        actor_perceive(actor, act_data, actor, act);
    }

    for (Actor *observer = actor->contents; observer; observer = observer->next_contents) {
        if (observer->dead) continue;
        actor_perceive(observer, act_data, actor, act);
    }

    if (actor->location_id) {
        Actor *location = world_find_actor(actor->location_id);

        actor_perceive(location, act_data, actor, act);

        for (Actor *observer = actor->contents; observer; observer = observer->next_contents) {
            if (observer == actor) continue;
            if (observer->dead) continue;

            actor_perceive(observer, act_data, actor, act);
        }
    }
}

static void actor_perceive(Actor *viewer, const ActData *act_data, Actor *actor, void *act) {
    ActPerceiveFn fn = NULL;

    if (!viewer->player) {
        fn = act_data->ai_perceive_fn;
    } else if (viewer->player->conn) {
        fn = act_data->player_perceive_fn;
    }

    if (fn) {
        (*fn)(viewer, actor, act);
    }
}