// movement.c
#include "movement.h"

#include "act.h"
#include "room.h"
#include "macros.h"
#include "player.h"
#include "world.h"
#include "log.h"
#include "interact.h"

#include <stdio.h>
#include <stddef.h>

static bool act_move_perform(Act *act);
static void act_move_player_perceive(Act *act, Actor *viewer);

bool act_move(Actor *actor, Direction direction) {
    ActMove act = (ActMove){
        .base = (Act) {
            .type = ACT_MOVE,
            .actor = actor,

            .perform_fn = act_move_perform,
            .player_perceive_fn = act_move_player_perceive
        },
        .status = ACT_MOVE_STATUS_OKAY,
        .direction = direction
    };

    act.from = world_find_actor(actor->location_id);
    if (!act.from || !act.from->room) {
        act.status = ACT_MOVE_STATUS_NOT_IN_ROOM;
        goto run;
    }

    act.exit = act.from->room->exits[act.direction];
    if (!act.exit || act.exit->to_room == INVALID_ACTOR_ID) {
        act.status = ACT_MOVE_STATUS_NO_EXIT;
        goto run;
    }

    act.to = world_find_actor(act.exit->to_room);
    if (!act.to || !act.to->room) {
        act.status = ACT_MOVE_STATUS_NO_EXIT;
        log_error("direction %s from %u: could not find room with id %u",
            direction_to_string(act.direction), actor->location_id, act.exit->to_room);
        goto run;
    }

    if (act.exit->closed) {
        act.status = ACT_MOVE_STATUS_EXIT_CLOSED;
        goto run;
    }

run:
    return act_run(&act.base);
}

static bool act_move_perform(Act *act) {
    ActMove *a = (ActMove*)act;
    CHECK(a->status >= ACT_MOVE_STATUS_OKAY && a->status <= ACT_MOVE_STATUS_EXIT_CLOSED);

    if (a->status != ACT_MOVE_STATUS_OKAY) {
        act_perceive_to(act, act->actor);
        return false;
    }

    CHECK(a->from && a->to && a->exit);

    act_perceive_at(act);
    actor_move_contents(a->from, act->actor, a->to);
    act_perceive_at_except(act, (Actor*[]){act->actor, NULL});

    act_examine_location(act->actor);

    return true;
}

static void act_move_player_perceive(Act *act, Actor *viewer) {
    CHECK(viewer->player != NULL);
    ActMove *a = (ActMove*)act;

    log_trace("Actor %d viewer %d", act->actor->id, viewer->id);

    switch (a->status) {
        case ACT_MOVE_STATUS_NOT_IN_ROOM:
            player_send(viewer->player, "You can't go anywhere from here.\n");
            break;
        case ACT_MOVE_STATUS_NO_EXIT:
            player_send(viewer->player, "You can't go that way.\n");
            break;
        case ACT_MOVE_STATUS_EXIT_CLOSED:
            player_sendf(viewer->player, "The %s is closed.\n", a->exit->keyword);
            break;
        case ACT_MOVE_STATUS_OKAY:
            if (viewer == act->actor) {
                player_sendf(viewer->player, "You move to the %s.\n", direction_to_string(a->direction));
            } else if (viewer->location_id == a->from->id) {
                player_sendf(viewer->player, "%s moves to the %s.\n", act->actor->appearance.name, direction_to_string(a->direction));
            } else if (viewer->location_id == a->to->id) {
                player_sendf(viewer->player, "%s arrives from the %s.\n", act->actor->appearance.name, direction_to_string(direction_reverse(a->direction)));
            } else {
                log_error("viewer %u not handled, viewer location %u, from location %u, to location %u", viewer->id, viewer->location_id, a->from->id, a->to->id);
            }
            break;
    }
}

void cmd_north(Actor *actor, const char *args) {
    UNUSED(args);
    act_move(actor, DIR_NORTH);
}
void cmd_south(Actor *actor, const char *args) {
    UNUSED(args);
    act_move(actor, DIR_SOUTH);
}
void cmd_east(Actor *actor, const char *args) {
    UNUSED(args);
    act_move(actor, DIR_EAST);
}
void cmd_west(Actor *actor, const char *args) {
    UNUSED(args);
    act_move(actor, DIR_WEST);
}
void cmd_up(Actor *actor, const char *args) {
    UNUSED(args);
    act_move(actor, DIR_UP);
}
void cmd_down(Actor *actor, const char *args) {
    UNUSED(args);
    act_move(actor, DIR_DOWN);
}
