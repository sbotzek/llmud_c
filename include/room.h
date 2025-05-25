// room.h
#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>
#include <stdbool.h>

#define ROOM_NAME_SIZE 64

typedef uint32_t RoomID;
#define INVALID_ROOM_ID 0

typedef struct Actor Actor;
typedef struct ActorNode ActorNode;
typedef struct World World;

typedef struct Room {
    RoomID id;
    char name[ROOM_NAME_SIZE];

    ActorNode *actors;
    // Add more properties later: description, exits, flags, etc.
} Room;

typedef struct RoomNode {
    Room *room;
    struct RoomNode *next;
} RoomNode;

void room_load_all(World *world);

void room_add_actor(Room *room, Actor *actor);

#endif //ROOM_H
