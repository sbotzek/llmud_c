// game_process.h
#ifndef GAME_PROCESS_H
#define GAME_PROCESS_H

typedef struct GameRules GameRules;
typedef struct World World;

typedef void (*GameProcessFn)(GameRules *rules, World *world);

typedef struct GameProcess {
    const char *name;
    GameProcessFn tick;
    unsigned int frequency;  // Run every N ticks (1 = every tick)
} GameProcess;

#endif
