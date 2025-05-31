// server.c
#define _POSIX_C_SOURCE 200112L
#include "server.h"

#include "game_process.h"
#include "world.h"
#include "tick.h"
#include "log.h"
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>

static void sleep_until_next_tick(struct timespec *next_tick) {
    // Don't sleep in test mode so we can run our tests as fast as possible.
    if (g_config.test_mode) {
        return;
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    long ms_remaining = (next_tick->tv_sec - now.tv_sec) * 1000 +
                        (next_tick->tv_nsec - now.tv_nsec) / 1000000;

    if (ms_remaining > 0) {
        struct timespec delay = {
            .tv_sec = ms_remaining / 1000,
            .tv_nsec = (ms_remaining % 1000) * 1000000
        };
        nanosleep(&delay, NULL);
    }

    // Advance the next tick time
    next_tick->tv_nsec += TICK_INTERVAL_MS * 1000000;
    while (next_tick->tv_nsec >= 1000000000) {
        next_tick->tv_nsec -= 1000000000;
        next_tick->tv_sec += 1;
    }
}

bool server_run(const GameProcess *processes, size_t process_count) {
    unsigned int tick = 0;
    struct timespec next_tick;
    clock_gettime(CLOCK_MONOTONIC, &next_tick);

    while (1) {
        log_trace("server_run: tick [%d]", tick);
        for (size_t i = 0; i < process_count; ++i) {
            if (tick % processes[i].frequency == 0) {
                log_trace("server_run: tick [%d], process [%s]", tick, processes[i].name);
                processes[i].tick();
            }
        }

        ++tick;
        sleep_until_next_tick(&next_tick);
    }

    return true;
}
