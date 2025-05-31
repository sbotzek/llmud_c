// room.h
#ifndef ROOM_H
#define ROOM_H

#include "actor.h"

typedef struct World World;

typedef enum {
    DIR_NORTH,
    DIR_SOUTH,
    DIR_EAST,
    DIR_WEST,
    DIR_UP,
    DIR_DOWN,
    DIR_COUNT
} Direction;


typedef struct Exit {
    Direction   dir;          // direction this exit faces
    ActorID     to_room;      // where it leads
    char       *keyword;      // optional: "oak door"
    bool        closed;       // false if open
    char       *description;  // what you see when looking that direction
} Exit;

typedef struct Room {
    Exit *exits[DIR_COUNT];   // exits indexed by direction
} Room;

const char *direction_to_string(Direction dir);
Direction    string_to_direction(const char *s);

#endif //ROOM_H
