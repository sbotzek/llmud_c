// player.c
#include "player.h"

#include <string.h>
#include <ctype.h>
#include <dirent.h> // For directory scanning

#include "macros.h"
#include "telnet_conn.h"
#include "account.h"
#include "actor.h"
#include "buffer.h"
#include "config.h"
#include "io.h"
#include "log.h"
#include "file_chunk.h"
#include "strutil.h"
#include "world.h"

Player* player_registry;

Player *player_new() {
    Player *player = calloc(1, sizeof(Player));
    CHECK_MSG(player != NULL, "player_new: malloc player failed");

    return player;
}

void player_free(Player *player) {
    CHECK(player != NULL);
    if (player->account) {
        account_free(player->account);
        player->account = NULL;
    }
    free(player);
}

void player_send(Player *player, const char *text) {
    CHECK(player != NULL);
    if (player->conn == NULL || !player->conn->connected) return;

    telnet_conn_write(player->conn, text);
}

void player_sendf(Player *player, const char *fmt, ...) {
    CHECK(player != NULL);
    if (player->conn == NULL || !player->conn->connected) return;

    va_list args;
    va_start(args, fmt);
    telnet_conn_vwritef(player->conn, fmt, args);
    va_end(args);
}

void player_handle_input(Player *player, const char *line) {
    CHECK(player != NULL);

    // For testing, makes it easier to cleanly disconnect a connection
    if (g_config.test_mode && strcmp(line, "killconn") == 0) {
        log_info("account killconn: %s as %s", player->conn->ip_string,
            (player->account == NULL ? NULL : player->account->username));
        if (player->actor) {
            world_remove_actor(player->actor);
            actor_free(player->actor);
            player->actor = NULL;
        }
        player_unregister(player);
        player_free(player);
        return;
    }
    player->input_handler(player, line);
}

char* player_name(Player *player) {
    CHECK(player != NULL);
    return player->account ? player->account->username : "?unknown?";
}

void player_register(Player *player) {
    CHECK(player != NULL);
    CHECK(player->next_in_registry == NULL);

    player->next_in_registry = player_registry;
    player_registry = player;
}

void player_unregister(Player *player) {
    CHECK(player != NULL);

    Player **pp = &player_registry;
    while (*pp) {
        if (*pp == player) {
            *pp = player->next_in_registry;
            break;
        }
        pp = &(*pp)->next_in_registry;
    }
}

Player *player_find_registered(const char *username) {
    CHECK(username != NULL);

    Player *player = player_registry;
    while (player) {
        if (player->account && strcmp(player->account->username, username) == 0) {
            return player;
        }
        player = player->next_in_registry;
    }
    return NULL;
}

void player_create_character(Player *player, const char *name) {
    CHECK(player != NULL);
    CHECK(player->account != NULL);
    CHECK(name != NULL);

    Actor actor;
    actor_init(&actor);
    actor.location_id = g_config.start_location_id;
    actor.appearance.name = str_copy(name);
    str_capitalize(actor.appearance.name);

    player_save_character(&actor);
    actor_cleanup(&actor);

    account_add_character(player->account, name);
    account_save(player->account);
}

void player_save_character(Actor *actor) {
    CHECK(actor != NULL);

    // Need to find a persistent actor to save this under
    ActorID location_id = actor->location_id;
    while (location_id > MAX_PERSISTENT_ACTOR_ID) {
        Actor *location = world_find_actor(location_id);
        if (location) {
            location_id = location->location_id;
        } else {
            log_error("player_save_character: Could not find actor %u to save character under (in actor %u)",
                location_id, actor->location_id);
            return;
        }
    }

    ensure_directory(DATA_DIR);
    ensure_directory(DATA_DIR "/pcs");

    Buffer path;
    buffer_init(&path, 0);
    buffer_printf(&path, DATA_DIR "/pcs/%s.pchar", actor->appearance.name);
    str_to_lower(path.data);

    FILE *fp = fopen(path.data, "w");
    CHECK_MSG(fp != NULL, "Failed to create character file: %s", path.data);

    // Write fields
    file_chunk_write_int_field(fp, "location_id", location_id);

    // Write sections
    appearance_write_section(&actor->appearance, fp, "appearance");

    fclose(fp);
    buffer_cleanup(&path);
}

Actor *player_load_character(const char *name) {
    CHECK(name != NULL);

    Buffer *path = buffer_new(128);
    buffer_printf(path, DATA_DIR "/pcs/%s.pchar", name);
    str_to_lower(path->data);

    FILE *fp = fopen(path->data, "r");
    buffer_free(path);

    if (!fp) {
        return NULL; // File doesn't exist.
    }

    Actor *actor = actor_new();
    FileChunkReader reader;
    file_chunk_reader_init(&reader, fp);

    // Read chunks
    while (file_chunk_read(&reader)) {
        FileChunk *chunk = &reader.chunk;

        if (chunk->type == FILE_CHUNK_SECTION_START) {
            if (strcmp(chunk->tag.data, "appearance") == 0) {
                // Hand off appearance parsing
                appearance_read_section(&actor->appearance, &reader, chunk->tag.data);
            } else {
                log_warn("Unknown section '%s' while loading character '%s'", chunk->tag.data, name);
                file_chunk_skip_section(&reader, chunk->tag.data);
            }
        } else if (chunk->type == FILE_CHUNK_SECTION_END) {
            // Should not happen at top level; log it
            log_warn("Unexpected section end '%s' while loading character '%s'", chunk->tag.data, name);
        } else if (chunk->type == FILE_CHUNK_FIELD) {
            if (strcmp(chunk->tag.data, "location_id") == 0) {
                actor->location_id = (ActorID)atoi(chunk->value.data);
            } else {
                log_warn("Unexpected field '%s' at top level while loading character '%s'", chunk->tag.data, name);
            }
        }
    }

    file_chunk_reader_cleanup(&reader);
    fclose(fp);

    // Validate appearance name (minimum field needed for a PC)
    if (actor->appearance.name == NULL) {
        log_error("player_load_character: Missing appearance.name for character '%s'", name);
        actor_free(actor);
        return NULL;
    }
    if (actor->location_id == INVALID_ACTOR_ID || actor->location_id > MAX_PERSISTENT_ACTOR_ID) {
        log_error("player_load_character: Invalid room id for character '%s'", name);
        actor_free(actor);
        return NULL;

    }

    actor->appearance.long_name = str_copy(actor->appearance.name);

    return actor;
}

bool player_validate_name(const char *name) {
    if (name == NULL) return false;

    size_t len = strlen(name);
    if (len < 3 || len > 10) return false;

    for (size_t i = 0; i < len; ++i) {
        if (!isalpha((unsigned char)name[i])) {
            return false;
        }
    }

    return true;
}

bool player_name_exists(const char *name) {
    CHECK(name != NULL);

    // First: check the player_registry (logged-in players)
    Player *player = player_registry;
    while (player) {
        if (player->account && account_has_character(player->account, name)) {
            return true;
        }
        player = player->next_in_registry;
    }

    // Second: check the pcs/ directory for stored characters
    ensure_directory(DATA_DIR);
    ensure_directory(DATA_DIR "/pcs");

    DIR *dir = opendir(DATA_DIR "/pcs");
    CHECK_MSG(dir != NULL, "Failed to open pcs directory");

    struct dirent *entry;
    size_t name_len = strlen(name);

    while ((entry = readdir(dir)) != NULL) {
        const char *filename = entry->d_name;
        size_t filename_len = strlen(filename);

        // Match: <name>.pchar
        if (filename_len == name_len + 6 && // ".pchar" = 6 chars
            strncmp(filename, name, name_len) == 0 &&
            strcmp(filename + name_len, ".pchar") == 0) {
            closedir(dir);
            return true;
            }
    }

    closedir(dir);
    return false;
}
