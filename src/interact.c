// interact.c
#include "interact.h"

#include "macros.h"
#include "world.h"
#include "actor.h"
#include "player.h"
#include "buffer.h"

static bool act_examine_location_perform(Act *act);
static void act_examine_location_player_perceive(Act *act, Actor *viewer);

void act_examine_location(Actor *actor) {
    ActExamineLocation act = (ActExamineLocation) {
        .base = (Act) {
            .type = ACT_EXAMINE_LOCATION,
            .actor = actor,

            .perform_fn = act_examine_location_perform,
            .player_perceive_fn = act_examine_location_player_perceive,
        },
    };

    act_run(&act.base);
}

bool act_examine_location_perform(Act *act) {
    act_perceive_to(act, act->actor);
    return true;
}

void act_examine_location_player_perceive(Act *act, Actor *viewer) {
    CHECK(viewer->player);

    Player *player = viewer->player;

    Actor *location = world_find_actor(act->actor->location_id);
    if (location == NULL) {
        player_send(player, "You are in nothingness.\n");
        return;
    }

    player_sendf(player, "%s\n", location->appearance.name);

    // Show contents
    for (Actor *contents = location->contents; contents; contents = contents->next_contents) {
        if (contents->dead) continue;
        if (player->actor == contents) continue;

        player_sendf(player, "You see: %s\n", contents->appearance.name);
    }

    // Show exits
    if (location->room) {
        char cbuf[256];
        Buffer buf;
        buffer_init(&buf, cbuf, sizeof(cbuf));

        buffer_append_str(&buf, "Exits: ");
        bool first = true;
        for (int i = 0; i < DIR_COUNT; ++i) {
            Exit *e = location->room->exits[i];
            if (!e) continue;

            if (!first) buffer_append_str(&buf, " ");
            first = false;

            const char *name = direction_to_string((Direction)i);
            if (e->closed) {
                buffer_appendf(&buf, "[%s]", name);
            } else {
                buffer_append_str(&buf, name);
            }
        }

        buffer_append_str(&buf, "\n");
        player_send(player, buf.data);
    }
}

static bool act_examine_direction_perform(Act *act);
static void act_examine_direction_player_perceive(Act *act, Actor *viewer);

void act_examine_direction(Actor *actor, Direction direction) {
    ActExamineDirection act = (ActExamineDirection) {
        .base = (Act) {
            .type = ACT_EXAMINE_DIRECTION,
            .actor = actor,

            .perform_fn = act_examine_direction_perform,
            .player_perceive_fn = act_examine_direction_player_perceive,
        },
        .direction = direction,
    };

    act.location = world_find_actor(actor->location_id);
    if (act.location && act.location->room) {
        act.exit = act.location->room->exits[direction];
    }

    act_run(&act.base);
}

bool act_examine_direction_perform(Act *act) {
    act_perceive_to(act, act->actor);
    return true;
}

void act_examine_direction_player_perceive(Act *act, Actor *viewer) {
    ActExamineDirection *a = (ActExamineDirection*)act;

    if (!a->exit) {
        player_sendf(viewer->player, "You see nothing special to the %s.\n", direction_to_string(a->direction));
    } else if (a->exit->keyword && a->exit->closed) {
        player_sendf(viewer->player, "The %s is closed.\n", a->exit->keyword);
    } else if (a->exit->keyword && !a->exit->closed) {
        player_sendf(viewer->player, "The %s is open.\n", a->exit->keyword);
    } else if (a->exit->closed) {
        player_sendf(viewer->player, "The way is closed.\n");
    } else {
        player_sendf(viewer->player, "You see nothing special to the %s.\n", direction_to_string(a->direction));
    }
}

void cmd_look(Actor *actor, const char *args) {
    if (args) {
        Direction dir = string_to_direction(args);
        if (dir != DIR_COUNT) {
            act_examine_direction(actor, dir);
        } else {
            if (actor->player) {
                player_sendf(actor->player, "You see no '%s' here.\n", args);
            }
        }
        return;
    }

    act_examine_location(actor);
}
