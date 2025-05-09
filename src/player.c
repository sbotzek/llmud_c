// player.c
#include "player.h"

#include <string.h>

#include "macros.h"
#include "telnet_conn.h"
#include "account.h"
#include "buffer.h"
#include "io.h"
#include "file_chunk.h"
#include "strutil.h"

Player* player_registry;

Player *player_new() {
    Player *player = calloc(1, sizeof(Player));
    CHECK_MSG(player != NULL, "player_new: malloc player failed");

    return player;
}

void player_free(Player *player) {
    CHECK(player != NULL);
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


void player_handle_input(Player *player, GameRules *rules, World *world, const char *line) {
    CHECK(player != NULL);

    player->input_handler(rules, world, player, line);
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

    ensure_directory(DATA_DIR);
    ensure_directory(DATA_DIR "/pcs");

    Buffer path;
    buffer_init(&path, 0);
    buffer_printf(&path, DATA_DIR "/pcs/%s.pchar", name);

    FILE *fp = fopen(path.data, "w");
    CHECK_MSG(fp != NULL, "Failed to create character file: %s", path.data);

    fprintf(fp, "#character\n");
    file_chunk_write_field(fp, "name", name);
    fprintf(fp, "#end character\n");

    fclose(fp);
    buffer_cleanup(&path);

    account_add_character(player->account, str_copy(name));
    account_save(player->account);
}
