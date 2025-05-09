// game_processes.h
#ifndef GAME_PROCESSES_H
#define GAME_PROCESSES_H

#include "game_process.h"

void event_gc_tick(GameRules *rules, World *world);

void telnet_listen_tick(GameRules *rules, World *world);
void telnet_read_tick(GameRules *rules, World *world);
void telnet_process_input_tick(GameRules *rules, World *world);
void telnet_flush_tick(GameRules *rules, World *world);
void telnet_gc_tick(GameRules *rules, World *world);
void buffer_gc_scratch_tick(GameRules *rules, World *world);

#endif // GAME_PROCESSES_H
