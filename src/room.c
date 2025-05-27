// room.c
#include "room.h"

#include <math.h>

#include "file_chunk.h"
#include "macros.h"
#include "log.h"
#include "world.h"
#include "io.h"
#include "strutil.h"
#include "actor.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define ROOMS_PATH DATA_DIR "/rooms.txt"

static Actor *read_one_room(FileChunkReader *r);

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
            Actor *actor = read_one_room(&r);
            if (actor) {
                world_add_actor(world, actor);
                log_info("Loaded room [%u]: %s", actor->id, actor->appearance.name);
            }
        } else {
            log_warn("Skipping unknown top-level section '%s'", r.chunk.tag.data);
            file_chunk_skip_section(&r, r.chunk.tag.data);
        }
    }

    file_chunk_reader_cleanup(&r);
    fclose(fp);
}

static Actor *read_one_room(FileChunkReader *r) {
    CHECK(r != NULL);

    Actor *actor = NULL;

    while (file_chunk_read(r)) {
        FileChunk *chunk = &r->chunk;

        if (chunk->type == FILE_CHUNK_SECTION_END &&
            strcmp(chunk->tag.data, "room") == 0) {
            break;
        }

        if (strcmp(chunk->tag.data, "id") == 0) {
            ActorID id = (ActorID)atoi(chunk->value.data);

            actor = actor_new_persistent(id);
            actor->room = calloc(1, sizeof(Room));
            CHECK_MSG(actor->room != NULL, "OOM allocating Room");
        } else if (strcmp(chunk->tag.data, "name") == 0) {
            actor->appearance.name = str_copy(chunk->value.data);
            actor->appearance.long_name = str_copy(chunk->value.data);
        } else if (strcmp(chunk->tag.data, "exit") == 0) {
            Exit *exit = calloc(1, sizeof(Exit));
            char *args = chunk->value.data;
            char word[1024];

            args = str_parse_word(args, word);
            exit->dir = string_to_direction(word);

            if (args) {
                args = str_parse_word(args, word);
                exit->to_room = (ActorID)atoi(word);
            }

            if (args) {
                args = str_parse_word(args, word);
                exit->open = strcmp(word, "true") == 0;
            }

            if (args) {
                exit->keyword = str_copy(args);
            }

            if (actor->room->exits[exit->dir] != NULL) {
                log_warn("Duplicate exit %d for %d.", exit->dir, actor->id);
                free(actor->room->exits[exit->dir]->keyword);
                actor->room->exits[exit->dir]->keyword = NULL;
                free(actor->room->exits[exit->dir]);
            }
            actor->room->exits[exit->dir] = exit;
        } else {
            log_warn("Unknown room field: %s", chunk->tag.data);
        }
    }

    if (actor == NULL) {
        log_error("Skipping invalid room (no id)");
        actor_free(actor);
        return NULL;
    }
    if (actor->appearance.name == NULL) {
        log_error("Skipping invalid room %d: missing name", actor->id);
        actor_free(actor);
        return NULL;

    }
    return actor;
}


const char *direction_to_string(Direction dir) {
    switch (dir) {
        case DIR_NORTH: return "north";
        case DIR_SOUTH: return "south";
        case DIR_EAST:  return "east";
        case DIR_WEST:  return "west";
        case DIR_UP:    return "up";
        case DIR_DOWN:  return "down";
        default:        return "";
    }
}

Direction string_to_direction(const char *s) {
    if (!s) return DIR_COUNT;
    if (strcmp(s, "north") == 0) return DIR_NORTH;
    if (strcmp(s, "south") == 0) return DIR_SOUTH;
    if (strcmp(s, "east")  == 0) return DIR_EAST;
    if (strcmp(s, "west")  == 0) return DIR_WEST;
    if (strcmp(s, "up")    == 0) return DIR_UP;
    if (strcmp(s, "down")  == 0) return DIR_DOWN;
    return DIR_COUNT;
}