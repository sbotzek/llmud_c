// movement.h
#ifndef MOVEMENT_H
#define MOVEMENT_H
#include "room.h"
#include "act.h"

typedef enum ActMoveStatus {
    ACT_MOVE_STATUS_OKAY,

    ACT_MOVE_STATUS_NOT_IN_ROOM,
    ACT_MOVE_STATUS_NO_EXIT,
    ACT_MOVE_STATUS_EXIT_CLOSED,
} ActMoveStatus;

typedef struct ActMove {
    Act base;

    ActMoveStatus status;

    // set before executing
    Direction direction;

    // set during prepare
    Actor *from;
    Exit *exit;
    Actor *to;
} ActMove;

bool act_move(Actor *actor, Direction direction);

void cmd_north(Actor *actor, const char *args);
void cmd_south(Actor *actor, const char *args);
void cmd_east(Actor *actor, const char *args);
void cmd_west(Actor *actor, const char *args);
void cmd_up(Actor *actor, const char *args);
void cmd_down(Actor *actor, const char *args);

#endif //MOVEMENT_H
