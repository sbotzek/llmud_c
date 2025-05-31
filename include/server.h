// server.h
#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct GameRules GameRules;
typedef struct GameProcess GameProcess;

bool server_run(const GameProcess *processes, size_t process_count);

#endif
