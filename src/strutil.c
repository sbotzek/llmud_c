// strutil.c
#include "strutil.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>

char *str_copy(const char *src) {
    CHECK(src);
    size_t len = strlen(src);
    char *copy = malloc(len + 1);
    CHECK_MSG(copy, "OOM duplicating string");
    memcpy(copy, src, len + 1);
    return copy;
}

// Returns true if 'word' is found as a space-separated token in 'list'
bool str_token_contains(const char *list, const char *word) {
    CHECK(list != NULL);
    CHECK(word != NULL);

    size_t wlen = strlen(word);
    const char *p = list;

    while (*p) {
        while (*p == ' ') ++p;
        if (strncmp(p, word, wlen) == 0 && (p[wlen] == ' ' || p[wlen] == '\0')) {
            return true;
        }
        while (*p && *p != ' ') ++p;
    }

    return false;
}
