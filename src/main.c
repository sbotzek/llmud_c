#include "server.h"
#include "world.h"
#include "game_rules.h"
#include "game_process.h"
#include "game_processes.h"

int main(void) {
    World world;
    GameRules rules;

    const GameProcess processes[] = {
        telnet_process(),
        // add more here
    };

    size_t process_count = sizeof(processes) / sizeof(GameProcess);

    return server_run(&rules, &world, processes, process_count) ? 0 : 1;
}
