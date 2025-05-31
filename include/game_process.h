// game_process.h
#ifndef GAME_PROCESS_H
#define GAME_PROCESS_H

typedef void (*GameProcessFn)();

typedef struct GameProcess {
    const char *name;
    GameProcessFn tick;
    unsigned int frequency;  // Run every N ticks (1 = every tick)
} GameProcess;

#endif
