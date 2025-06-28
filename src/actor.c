// actor.c
#include "actor.h"

#include "player.h"
#include "macros.h"
#include "log.h"
#include "file_chunk.h"
#include "room.h"
#include <string.h>

static ActorID on_actor_id = MAX_PERSISTENT_ACTOR_ID + 1;

#define MAX_ACTOR_ID UINT32_MAX
#define WARN_ON_ACTOR_ID (UINT32_MAX / 2)

static void actor_init_internal(Actor *actor);

void actor_init_persistent(Actor *actor, ActorID id) {
    CHECK(id <= MAX_PERSISTENT_ACTOR_ID && id != INVALID_ACTOR_ID);
    actor_init_internal(actor);
    actor->id = id;
}


void actor_init(Actor *actor) {
    actor_init_internal(actor);
    actor->id = on_actor_id++;
}

static void actor_init_internal(Actor *actor) {
    CHECK(actor != NULL);

    *actor = (Actor){
        .id = INVALID_ACTOR_ID,
        .location_id = INVALID_ACTOR_ID,
        .dead = true
    };
    appearance_init(&actor->appearance);

    if (on_actor_id > WARN_ON_ACTOR_ID) {
        if (on_actor_id == MAX_ACTOR_ID) {
            log_fatal("Actor ID reached max value: %u", on_actor_id);
        } else {
            log_warn("Actor ID reached warn level: %u", on_actor_id);
        }
    }
}

void actor_cleanup(Actor *actor) {
    CHECK(actor != NULL);

    if (actor->player) {
        CHECK_MSG(actor->player->actor == actor,
                  "actor-player mismatch: actor->id=%u, player->actor->id=%u",
                  actor->id,
                  actor->player->actor ? actor->player->actor->id : INVALID_ACTOR_ID);
        actor->player->actor = NULL;
        actor->player = NULL;
    }

    appearance_cleanup(&actor->appearance);
}

Actor* actor_new() {
    Actor *actor = calloc(1, sizeof(Actor));
    CHECK_MSG(actor != NULL, "actor_new: malloc actor failed");

    actor_init(actor);

    return actor;
}

Actor* actor_new_persistent(ActorID id) {
    Actor *actor = calloc(1, sizeof(Actor));
    CHECK_MSG(actor != NULL, "actor_new: malloc actor failed");

    actor_init_persistent(actor, id);

    return actor;
}

void actor_free(Actor* actor) {
    CHECK(actor != NULL);
    actor_cleanup(actor);
    free(actor);

}
void actor_add_contents(Actor *location, Actor *contents) {
    CHECK(location != NULL);
    CHECK(contents != NULL);
    CHECK(contents->location_id == INVALID_ACTOR_ID);

    contents->next_contents = location->contents;
    location->contents = contents;
    contents->location_id = location->id;
}

void actor_remove_contents(Actor *location, Actor *contents) {
    CHECK(location != NULL);
    CHECK(contents != NULL);
    CHECK(contents->location_id == location->id);

    Actor **pp = &location->contents;
    while (*pp) {
        if (*pp == contents) {
            *pp = (*pp)->next_contents;
            contents->location_id = INVALID_ACTOR_ID;
            return;
        }
        pp = &(*pp)->next_contents;
    }
}

void actor_move_contents(Actor *location, Actor *contents, Actor *new_location) {
    CHECK(location      != NULL);
    CHECK(contents      != NULL);
    CHECK(new_location  != NULL);
    CHECK(contents->location_id == location->id);
    CHECK(new_location->id != INVALID_ACTOR_ID);

    if (location->id == new_location->id) {
        return;
    }

    actor_remove_contents(location, contents);
    actor_add_contents(new_location, contents);
}

// — Serialization

void actor_save(Actor *actor, FILE *fp, const char *section_name) {
    CHECK(actor != NULL);
    CHECK(fp != NULL);

    bool use_section = section_name && section_name[0];
    if (use_section) {
        file_chunk_write_section_start(fp, section_name);
    }

    // Write actor fields
    if (actor->id != INVALID_ACTOR_ID && actor->id <= MAX_PERSISTENT_ACTOR_ID) {
        file_chunk_write_int_field(fp, "id", actor->id);
    }
    if (actor->location_id != INVALID_ACTOR_ID) {
        file_chunk_write_int_field(fp, "location_id", actor->location_id);
    }
    if (actor->dead) {
        file_chunk_write_field(fp, "dead", "true");
    }

    // Write components
    appearance_write_section(&actor->appearance, fp, "appearance");
    
    if (actor->room) {
        room_write_section(actor->room, fp, "room");
    }

    if (use_section) {
        file_chunk_write_section_end(fp, section_name);
    }
}

Actor *actor_load(FileChunkReader *r, const char *section_name) {
    CHECK(r != NULL);

    bool use_section = section_name && section_name[0];
    Actor *actor = NULL;

    while (file_chunk_read(r)) {
        FileChunk *chunk = &r->chunk;

        if (use_section && chunk->type == FILE_CHUNK_SECTION_END &&
            strcmp(chunk->tag.data, section_name) == 0) {
            break;
        }
        if (!use_section && chunk->type == FILE_CHUNK_SECTION_END) {
            log_warn("Unexpected section end '%s' at top level", chunk->tag.data);
            if (actor) {
                actor_free(actor);
            }
            return NULL;
        }

        if (chunk->type == FILE_CHUNK_FIELD && strcmp(chunk->tag.data, "id") == 0) {
            if (actor != NULL) {
                log_warn("Found duplicate actor ID");
                actor_free(actor);
            }
            ActorID id = (ActorID)atoi(chunk->value.data);
            actor = actor_new_persistent(id);
            continue;
        }

        if (actor == NULL) {
            actor = actor_new();
        }

        if (chunk->type == FILE_CHUNK_FIELD) {
            if (strcmp(chunk->tag.data, "location_id") == 0) {
                actor->location_id = (ActorID)atoi(chunk->value.data);
            } else if (strcmp(chunk->tag.data, "dead") == 0) {
                actor->dead = strcmp(chunk->value.data, "true") == 0;
            } else {
                log_warn("Unknown actor field '%s'", chunk->tag.data);
            }
        } else if (chunk->type == FILE_CHUNK_SECTION_START) {
            if (strcmp(chunk->tag.data, "appearance") == 0) {
                appearance_read_section(&actor->appearance, r, "appearance");
            } else if (strcmp(chunk->tag.data, "room") == 0) {
                actor->room = calloc(1, sizeof(Room));
                CHECK_MSG(actor->room != NULL, "OOM allocating Room");
                room_read_section(actor->room, r, "room");
            } else {
                log_warn("Unknown actor section '%s'", chunk->tag.data);
                file_chunk_skip_section(r, chunk->tag.data);
            }
        }
        if (!use_section && feof(r->fp)) {
            break;
        }
    }

    return actor;
}
