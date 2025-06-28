// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "actor_id.h"

// Global configuration options for the MUD.
typedef struct Config {
    bool     test_mode;         // Run in test mode: no sleeps, no password hashing
    uint16_t port;              // Listening port (e.g., 4000)
    ActorID start_location_id;
} Config;

// The active global config.
extern Config g_config;

// Initializes the config from argv.
void config_init(int argc, char **argv);

#endif // CONFIG_H
