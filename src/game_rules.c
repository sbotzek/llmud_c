#include "game_rules.h"

#include "macros.h"

GameRules *game_rules_create() {
    GameRules *rules = calloc(1, sizeof(GameRules));
    CHECK_MSG(rules != NULL, "game_rules_create: malloc player failed");

    return rules;
}
