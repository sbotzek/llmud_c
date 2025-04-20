#include <stdio.h>
#include "server.h"

int main() {
    int port = 4000;

    if (!server_start(port)) {
        fprintf(stderr, "Failed to start server on port %d\n", port);
        return 1;
    }

    return 0;
}
