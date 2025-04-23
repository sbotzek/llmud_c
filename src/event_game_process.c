#include "event.h"
#include "macros.h"
#include "game_process.h"
#include "tick.h"

void event_cleanup_subscriptions_shim(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);

    event_cleanup_subscriptions();
}

GameProcess event_cleanup_subscriptions_process(void) {
    return (GameProcess){
        .name = "event cleanup subscriptions",
        .tick = event_cleanup_subscriptions_shim,
        .frequency = TICKS_PER_MINUTE
    };
}
