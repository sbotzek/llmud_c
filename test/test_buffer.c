// test_buffer.c
#include "buffer.h"
#include "test_macros.h"
#include <string.h>
#include <stdlib.h>

TEST(test_buffer_basic) {
    DynamicBuffer buf;
    dbuffer_init(&buf, 4);
    dbuffer_append_str(&buf, "hello");
    ASSERT(strcmp(buf.data, "hello") == 0);
    ASSERT(buf.length == 5);

    dbuffer_append_str(&buf, " world");
    ASSERT(strcmp(buf.data, "hello world") == 0);

    dbuffer_clear(&buf);
    ASSERT(buf.length == 0);
    ASSERT(strcmp(buf.data, "") == 0);

    dbuffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_printf_and_appendf) {
    DynamicBuffer buf;
    dbuffer_init(&buf, 8);
    dbuffer_printf(&buf, "%s = %d", "x", 10);
    ASSERT(strcmp(buf.data, "x = 10") == 0);

    dbuffer_appendf(&buf, ", %s", "done");
    ASSERT(strcmp(buf.data, "x = 10, done") == 0);

    dbuffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_trim) {
    DynamicBuffer buf;
    dbuffer_init(&buf, 16);
    dbuffer_append_str(&buf, "   trimmed \t\n");
    dbuffer_trim(&buf);
    ASSERT(strcmp(buf.data, "trimmed") == 0);

    dbuffer_clear(&buf);
    dbuffer_append_str(&buf, "   ");
    dbuffer_trim(&buf);
    ASSERT(strcmp(buf.data, "") == 0);
    ASSERT(buf.length == 0);

    dbuffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_large_format) {
    DynamicBuffer buf;
    dbuffer_init(&buf, 1);
    dbuffer_printf(&buf, "%s", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    ASSERT(strstr(buf.data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") != NULL);
    dbuffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_reserve_growth) {
    DynamicBuffer buf;
    dbuffer_init(&buf, 8);
    size_t initial_capacity = buf.capacity;
    dbuffer_reserve(&buf, initial_capacity + 100);
    ASSERT(buf.capacity >= initial_capacity + 100);
    dbuffer_cleanup(&buf);
    return 0;
}

#include "test_macros.h"

int main(void) {
    // Dynamic buffer tests
    RUN_TEST(test_buffer_basic);
    RUN_TEST(test_buffer_printf_and_appendf);
    RUN_TEST(test_buffer_trim);
    RUN_TEST(test_buffer_large_format);
    RUN_TEST(test_buffer_reserve_growth);

    printf("All buffer tests passed.\n");
    return 0;
}
