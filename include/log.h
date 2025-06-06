// log.h
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>

// Log level definitions
#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

// Default log level if none specified at compile time
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// Internal function declaration
void log_internal(int level, const char *level_str,
                  const char *file, const char *func, int line,
                  const char *fmt, ...);

// Filtered macro dispatch
#define LOG(level, level_str, ...) \
    do { \
        if ((level) <= LOG_LEVEL) \
            log_internal((level), (level_str), __FILE__, __func__, __LINE__, __VA_ARGS__); \
    } while (0)

// Public log macros
#define log_fatal(...) do { LOG(LOG_LEVEL_FATAL, "FATAL", __VA_ARGS__); abort(); } while (0)
#define log_error(...) LOG(LOG_LEVEL_ERROR, "ERROR", __VA_ARGS__)
#define log_warn(...)  LOG(LOG_LEVEL_WARN,  "WARN",  __VA_ARGS__)
#define log_info(...)  LOG(LOG_LEVEL_INFO,  "INFO",  __VA_ARGS__)
#define log_debug(...) LOG(LOG_LEVEL_DEBUG, "DEBUG", __VA_ARGS__)
#define log_trace(...) LOG(LOG_LEVEL_TRACE, "TRACE", __VA_ARGS__)

#endif // LOG_H
