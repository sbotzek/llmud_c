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

        if (chunk->type != FILE_CHUNK_FIELD) {
            log_warn("Unexpected chunk in room section at line %d", r->line_number);
            continue;
        }

        if (strcmp(chunk->tag.data, "id") == 0) {
            ActorID id = (ActorID)atoi(chunk->value.data);

            actor = actor_new_persistent(id);
            actor->room = calloc(1, sizeof(Room));
            CHECK_MSG(actor->room != NULL, "OOM allocating Room");
        } else if (strcmp(chunk->tag.data, "name") == 0) {
            actor->appearance.name = str_copy(chunk->value.data);
            actor->appearance.long_name = str_copy(chunk->value.data);
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