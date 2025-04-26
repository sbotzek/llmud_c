#include "server.h"
#include "world.h"
#include "game_rules.h"
#include "game_process.h"
#include "game_processes.h"

int main(void) {
    World world;
    GameRules rules;

    const GameProcess processes[] = {
        event_cleanup_subscriptions_process(),
        telnet_listen_process(),
        telnet_read_process(),
    };

    size_t process_count = sizeof(processes) / sizeof(GameProcess);

    return server_run(&rules, &world, processes, process_count) ? 0 : 1;
}
