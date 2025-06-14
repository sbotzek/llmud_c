// test_buffer.c
#include "buffer.h"
#include "test_macros.h"
#include <string.h>
#include <stdlib.h>

TEST(test_buffer_basic) {
    Buffer buf;
    buffer_init(&buf, 4);
    buffer_append_str(&buf, "hello");
    ASSERT(strcmp(buf.data, "hello") == 0);
    ASSERT(buf.length == 5);

    buffer_append_str(&buf, " world");
    ASSERT(strcmp(buf.data, "hello world") == 0);

    buffer_clear(&buf);
    ASSERT(buf.length == 0);
    ASSERT(strcmp(buf.data, "") == 0);

    buffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_printf_and_appendf) {
    Buffer buf;
    buffer_init(&buf, 8);
    buffer_printf(&buf, "%s = %d", "x", 10);
    ASSERT(strcmp(buf.data, "x = 10") == 0);

    buffer_appendf(&buf, ", %s", "done");
    ASSERT(strcmp(buf.data, "x = 10, done") == 0);

    buffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_trim) {
    Buffer buf;
    buffer_init(&buf, 16);
    buffer_append_str(&buf, "   trimmed \t\n");
    buffer_trim(&buf);
    ASSERT(strcmp(buf.data, "trimmed") == 0);

    buffer_clear(&buf);
    buffer_append_str(&buf, "   ");
    buffer_trim(&buf);
    ASSERT(strcmp(buf.data, "") == 0);
    ASSERT(buf.length == 0);

    buffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_large_format) {
    Buffer buf;
    buffer_init(&buf, 1);
    buffer_printf(&buf, "%s", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    ASSERT(strstr(buf.data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ") != NULL);
    buffer_cleanup(&buf);
    return 0;
}

TEST(test_buffer_reserve_growth) {
    Buffer buf;
    buffer_init(&buf, 8);
    size_t initial_capacity = buf.capacity;
    buffer_reserve(&buf, initial_capacity + 100);
    ASSERT(buf.capacity >= initial_capacity + 100);
    buffer_cleanup(&buf);
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
