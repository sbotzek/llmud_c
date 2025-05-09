// io.c

#include "io.h"

#include "macros.h"

#include <sys/stat.h> /* for mkdir */
#include <errno.h>
#include <stdbool.h>
#include <string.h>

void ensure_directory(const char *path) {
    if (mkdir(path, 0755) != 0) {
        if (errno != EEXIST) {
            CHECK_MSG(false, "mkdir '%s' failed: %s", path, strerror(errno));
        }
    }
}

