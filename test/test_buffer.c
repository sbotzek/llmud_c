// test_buffer.c
#include "buffer.h"
#include "test_macros.h"
#include <string.h>

TEST(test_buffer_basic) {
    char data[64];
    Buffer buf = buffer_init(data, sizeof(data));

    buffer_append_str(&buf, "hello");
    ASSERT(strcmp(buf.data, "hello") == 0);

    buffer_appendf(&buf, " %s", "world");
    ASSERT(strcmp(buf.data, "hello world") == 0);

    buffer_clear(&buf);
    ASSERT(buf.length == 0);
    ASSERT(strcmp(buf.data, "") == 0);

    return 0;
}

TEST(test_buffer_overflow_truncates) {
    char data[16];
    Buffer buf = buffer_init(data, sizeof(data));

    buffer_append_str(&buf, "12345678901234567890");
    ASSERT(buf.length < sizeof(data));  // should truncate before overflow
    ASSERT(buf.data[buf.length] == '\0');
    ASSERT(buf.overflow);

    return 0;
}

TEST(test_buffer_trim) {
    char data[64];
    Buffer buf = buffer_init(data, sizeof(data));

    buffer_append_str(&buf, "   trim this   \n");
    buffer_trim(&buf);
    ASSERT(strcmp(buf.data, "trim this") == 0);

    buffer_clear(&buf);
    buffer_append_str(&buf, "\n \t");
    buffer_trim(&buf);
    ASSERT(buf.length == 0);
    ASSERT(strcmp(buf.data, "") == 0);

    return 0;
}

TEST(test_buffer_printf_long) {
    char data[64];
    Buffer buf = buffer_init(data, sizeof(data));

    buffer_printf(&buf, "%d + %d = %d", 2, 2, 4);
    ASSERT(strcmp(buf.data, "2 + 2 = 4") == 0);
    return 0;
}

int main(void) {
    // Static buffer tests
    RUN_TEST(test_buffer_basic);
    RUN_TEST(test_buffer_overflow_truncates);
    RUN_TEST(test_buffer_trim);
    RUN_TEST(test_buffer_printf_long);

    printf("All static buffer tests passed.\n");
    return 0;
}
