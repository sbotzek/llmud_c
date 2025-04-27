#include "player.h"

#include "macros.h"
#include "telnet_conn.h"

Player *player_create() {
    Player *player = malloc(sizeof(Player));
    CHECK_MSG(player != NULL, "player_create: malloc player failed");

    return player;
}

void player_destroy(Player *player) {
    CHECK(player != NULL);
    free(player);
}

void player_send(Player *player, const char *text) {
    CHECK(player != NULL);
    if (player->conn == NULL) return;

    telnet_conn_write(player->conn, text);
}

void player_sendf(Player *player, const char *fmt, ...) {
    CHECK(player != NULL);
    if (player->conn == NULL) return;

    va_list args;
    va_start(args, fmt);
    telnet_conn_vwritef(player->conn, fmt, args);
    va_end(args);
}


void player_handle_input(Player *player, GameRules *rules, World *world, const char *line) {
    CHECK(player != NULL);
    CHECK(player->conn != NULL);

    player->input_handler(rules, world, player, line);
}
