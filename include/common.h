#ifndef COMMON_H
#define COMMON_H

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

#define CHECK_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "[CHECK FAILED] %s\n", (msg)); \
            fprintf(stderr, "  Condition: %s\n", #cond); \
            fprintf(stderr, "  Location : %s:%d\n", __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)

#endif // COMMON_H
