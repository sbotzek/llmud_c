// room.c
#include "room.h"

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

void room_load_all(void) {
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
            Actor *actor = actor_load(&r, "room");
            if (actor) {
                world_add_actor(actor);
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

const char *direction_to_string(Direction dir) {
    switch (dir) {
        case DIR_NORTH: return "north";
        case DIR_SOUTH: return "south";
        case DIR_EAST:  return "east";
        case DIR_WEST:  return "west";
        case DIR_UP:    return "up";
        case DIR_DOWN:  return "down";
        default:        return "!INVALID!";
    }
}

Direction direction_reverse(Direction direction) {
    switch (direction) {
        case DIR_NORTH: return DIR_SOUTH;
        case DIR_SOUTH: return DIR_NORTH;
        case DIR_EAST:  return DIR_WEST;
        case DIR_WEST:  return DIR_EAST;
        case DIR_UP:    return DIR_DOWN;
        case DIR_DOWN:  return DIR_UP;
        default:
            log_error("direction_reverse: invalid direction %u, returning same", direction);
            return direction;
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

// — Serialization

void room_write_section(const Room *room, FILE *fp, const char *section) {
    CHECK(room != NULL);
    CHECK(fp != NULL);

    file_chunk_write_section_start(fp, section);

    // Write all exits
    for (int dir = 0; dir < DIR_COUNT; dir++) {
        Exit *exit = room->exits[dir];
        if (exit) {
            char args[256];
            snprintf(args, sizeof(args), "%s %u %s", 
                    direction_to_string(exit->dir),
                    exit->to_room,
                    exit->closed ? "true" : "false");
            
            if (exit->keyword) {
                // Append keyword if it exists
                size_t len = strlen(args);
                snprintf(args + len, sizeof(args) - len, " %s", exit->keyword);
            }
            
            file_chunk_write_section_inline(fp, "exit", args);
        }
    }

    file_chunk_write_section_end(fp, section);
}

void room_read_section(Room *room, FileChunkReader *r, const char *section) {
    CHECK(room != NULL);
    CHECK(r != NULL);

    while (file_chunk_read(r)) {
        FileChunk *chunk = &r->chunk;

        if (chunk->type == FILE_CHUNK_SECTION_END &&
            strcmp(chunk->tag.data, section) == 0) {
            return;
        }

        if (chunk->type == FILE_CHUNK_SECTION_START &&
            strcmp(chunk->tag.data, "exit") == 0) {
            Exit *exit = calloc(1, sizeof(Exit));
            CHECK_MSG(exit != NULL, "OOM allocating Exit");
            
            char *args = chunk->value.data;
            char word[1024];

            args = str_parse_word(args, word, sizeof(word));
            exit->dir = string_to_direction(word);

            if (args) {
                args = str_parse_word(args, word, sizeof(word));
                exit->to_room = (ActorID)atoi(word);
            }

            if (args) {
                args = str_parse_word(args, word, sizeof(word));
                exit->closed = strcmp(word, "true") == 0;
            }

            if (args) {
                exit->keyword = str_copy(args);
            }

            if (room->exits[exit->dir] != NULL) {
                log_warn("Duplicate exit %d, replacing existing", exit->dir);
                free(room->exits[exit->dir]->keyword);
                free(room->exits[exit->dir]);
            }
            room->exits[exit->dir] = exit;
        } else if (chunk->type != FILE_CHUNK_FIELD) {
            log_warn("Unexpected chunk type in room section at line %d", r->line_number);
        }
    }

    log_warn("Unterminated room section");
}