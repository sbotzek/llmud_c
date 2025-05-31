// game_processes.h
#ifndef GAME_PROCESSES_H
#define GAME_PROCESSES_H

void buffer_gc_scratch(void);
void telnet_listen_tick(void);
void telnet_read_tick(void);
void telnet_process_input_tick(void);
void telnet_flush_tick(void);
void telnet_gc_tick(void);
void buffer_gc_scratch_tick(void);

#endif // GAME_PROCESSES_H
