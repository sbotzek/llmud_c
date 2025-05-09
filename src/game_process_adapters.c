// game_process_adapters.c
/*
 * Contains adapters for functions that need to be called as game processes, but don't take the proper function
 * arguments.
 */
#include "game_processes.h"
#include "buffer.h"
#include "macros.h"

void buffer_gc_scratch_tick(GameRules *rules, World *world) {
    UNUSED(rules);
    UNUSED(world);

    buffer_gc_scratch();
}
