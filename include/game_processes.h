#ifndef GAME_PROCESSES_H
#define GAME_PROCESSES_H

#include "game_process.h"

GameProcess telnet_process(void);
GameProcess event_cleanup_subscriptions_process(void);

#endif // GAME_PROCESSES_H
