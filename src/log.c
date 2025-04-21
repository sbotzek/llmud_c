#include "log.h"
#include <stdarg.h>
#include "macros.h"

void log_internal(int level, const char *level_str,
                  const char *file, int line,
                  const char *fmt, ...) {
    UNUSED(level); // in case you want to log level-specific info later

    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[%s] %s:%d: ", level_str, file, line);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}
