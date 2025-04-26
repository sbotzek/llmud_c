#ifndef GAME_PROCESSES_H
#define GAME_PROCESSES_H

#include "game_process.h"

void event_cleanup_tick(GameRules *rules, World *world);

void telnet_listen_tick(GameRules *rules, World *world);
void telnet_read_tick(GameRules *rules, World *world);

void client_flush_tick(GameRules *rules, World *world);

#endif // GAME_PROCESSES_H
