// test_macros.h
#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#include <stdio.h>
#include <stdlib.h>

#define TEST(name) static int name(void)
#define RUN_TEST(fn) do { \
    printf("[TEST] %s...\n", #fn); \
    if ((fn)() != 0) { \
        fprintf(stderr, "[FAIL] %s\n", #fn); \
        return 1; \
    } \
    printf("[PASS] %s\n", #fn); \
} while (0)

#define ASSERT(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "ASSERT FAILED: %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

#endif // TEST_MACROS_H
