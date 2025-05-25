// room.c
#include "room.h"
#include "file_chunk.h"
#include "macros.h"
#include "log.h"
#include "world.h"
#include "io.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/macros.h"

#define ROOMS_PATH DATA_DIR "/rooms.txt"

static Room *read_one_room(FileChunkReader *r);

void room_load_all(World *world) {
    CHECK(world != NULL);
    ensure_directory(DATA_DIR);

    FILE *fp = fopen(ROOMS_PATH, "r");
    if (!fp) {
        log_warn("No room file found at %s", ROOMS_PATH);
        return;
    }

    FileChunkReader r;
    file_chunk_reader_init(&r, fp);

    while (file_chunk_read(&r)) {
        if (r.chunk.type == FILE_CHUNK_SECTION_START &&
            strcmp(r.chunk.tag.data, "room") == 0) {
            Room *room = read_one_room(&r);
            if (room) {
                world_add_room(world, room);
                log_info("Loaded room [%u]: %s", room->id, room->name);
            }
        } else {
            log_warn("Skipping unknown top-level section '%s'", r.chunk.tag.data);
            file_chunk_skip_section(&r, r.chunk.tag.data);
        }
    }

    file_chunk_reader_cleanup(&r);
    fclose(fp);
}

static Room *read_one_room(FileChunkReader *r) {
    CHECK(r != NULL);

    Room *room = calloc(1, sizeof(Room));
    CHECK_MSG(room != NULL, "OOM allocating Room");

    bool got_id = false;
    bool got_name = false;

    while (file_chunk_read(r)) {
        FileChunk *chunk = &r->chunk;

        if (chunk->type == FILE_CHUNK_SECTION_END &&
            strcmp(chunk->tag.data, "room") == 0) {
            break;
        }

        if (chunk->type != FILE_CHUNK_FIELD) {
            log_warn("Unexpected chunk in room section at line %d", r->line_number);
            continue;
        }

        if (strcmp(chunk->tag.data, "id") == 0) {
            room->id = (RoomID)atoi(chunk->value.data);
            got_id = true;
        } else if (strcmp(chunk->tag.data, "name") == 0) {
            strncpy(room->name, chunk->value.data, ROOM_NAME_SIZE - 1);
            room->name[ROOM_NAME_SIZE - 1] = '\0';
            got_name = true;
        } else {
            log_warn("Unknown room field: %s", chunk->tag.data);
        }
    }

    if (!got_id || !got_name) {
        log_error("Skipping invalid room (missing id or name)");
        free(room);
        return NULL;
    }

    return room;
}

void room_add_actor(Room *room, Actor *actor) {
    CHECK(actor->in_room_id == INVALID_ROOM_ID);

    actor->in_room_id = room->id;

    ActorNode *node = calloc(1, sizeof(ActorNode));

    node->actor = actor;
    node->next = room->actors;
    room->actors = node;
}

void room_remove_actor(Room *room, Actor *actor) {
    ActorNode **pp = &room->actors;

    while (*pp) {
        ActorNode *node = *pp;
        if (node->actor == actor) {
            *pp = node->next;
            node->actor = NULL;
            actor->in_room_id = INVALID_ROOM_ID;
            free(node);
            return;
        }
    }

    CHECK_MSG(false, "room_remove_actor: actor not found in room");
}

void room_move_actor(Room *room, Actor *actor, Room *new_room) {
    room_remove_actor(room, actor);
    room_add_actor(new_room, actor);
}