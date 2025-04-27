// macros.h
#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[CHECK FAILED]\n"); \
            fprintf(stderr, "  Condition: %s\n", #cond); \
            fprintf(stderr, "  Location : %s:%d\n", __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)

#define CHECK_MSG(cond, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[CHECK FAILED] "); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            fprintf(stderr, "  Condition: %s\n", #cond); \
            fprintf(stderr, "  Location : %s:%d\n", __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)

#endif // MACROS_H
