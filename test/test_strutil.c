// test_strutil.c
#include "strutil.h"
#include "test_macros.h"
#include <string.h>
#include <stdlib.h>

TEST(test_str_copy) {
    char *copy = str_copy("hello");
    ASSERT(strcmp(copy, "hello") == 0);
    free(copy);
    return 0;
}

TEST(test_str_token_contains) {
    ASSERT(str_token_contains("foo bar baz", "bar") == true);
    ASSERT(str_token_contains("foo bar baz", "qux") == false);
    ASSERT(str_token_contains("  apple banana cherry ", "banana") == true);
    ASSERT(str_token_contains("apple banana", "apple") == true);
    ASSERT(str_token_contains("apple banana", "ban") == false);
    return 0;
}

int main(void) {
    RUN_TEST(test_str_copy);
    RUN_TEST(test_str_token_contains);
    printf("All strutil tests passed.\n");
    return 0;
}
