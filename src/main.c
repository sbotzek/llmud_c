#include "server.h"
#include "world.h"
#include "game_rules.h"
#include "game_process.h"
#include "game_processes.h"

int main(void) {
    World world;
    GameRules rules;

    const GameProcess processes[] = {
        {.name="event.cleanup",          .tick=event_cleanup_tick,         .frequency=1},
        {.name="telnet.listen",          .tick=telnet_listen_tick,         .frequency=1},
        {.name="telnet.read",            .tick=telnet_read_tick,           .frequency=1},
        {.name="telnet.process_input",   .tick=telnet_process_input_tick,  .frequency=1},
        {.name="telnet.flush",           .tick=telnet_flush_tick,          .frequency=1},
    };

    size_t process_count = sizeof(processes) / sizeof(GameProcess);

    return server_run(&rules, &world, processes, process_count) ? 0 : 1;
}
