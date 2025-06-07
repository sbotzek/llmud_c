// interact.h
#ifndef INTERACT_H
#define INTERACT_H
#include "act.h"
#include "room.h"

typedef struct ActExamineLocation {
    Act base;
} ActExamineLocation;

typedef struct ActExamineDirection {
    Act base;

    Direction direction;
    Actor *location;
    Exit *exit;
} ActExamineDirection;

typedef struct Actor Actor;

void act_examine_location(Actor *actor);
void act_examine_direction(Actor *actor, Direction direction);

void cmd_look(Actor *actor, const char *args);

#endif // INTERACT_H
