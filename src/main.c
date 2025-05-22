// main.c
#include "server.h"
#include "world.h"
#include "config.h"
#include "game_rules.h"
#include "game_process.h"
#include "game_processes.h"

int main(int argc, char **argv) {
    config_init(argc, argv);

    World *world = world_new();
    GameRules *rules = game_rules_new();

    const GameProcess processes[] = {
        {.name="telnet.listen",          .tick=telnet_listen_tick,         .frequency=1},
        {.name="telnet.read",            .tick=telnet_read_tick,           .frequency=1},
        {.name="telnet.process_input",   .tick=telnet_process_input_tick,  .frequency=1},
        {.name="telnet.flush",           .tick=telnet_flush_tick,          .frequency=1},
        {.name="telnet.gc",              .tick=telnet_gc_tick,             .frequency=1},
        {.name="buffer.gc_scratch",      .tick=buffer_gc_scratch_tick,     .frequency=1},
    };

    size_t process_count = sizeof(processes) / sizeof(GameProcess);

    return server_run(rules, world, processes, process_count) ? 0 : 1;
}
