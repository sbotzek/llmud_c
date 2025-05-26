// config.c
#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Config g_config = {
    .test_mode = false,
    .port = 4000,
    .start_location_id = 1
};

void config_init(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--test-mode") == 0) {
            g_config.test_mode = true;
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            g_config.port = (uint16_t)atoi(argv[++i]);
            if (g_config.port == 0) {
                fprintf(stderr, "Invalid port number after --port\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            exit(1);
        }
    }
}
