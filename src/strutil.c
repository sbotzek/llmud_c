// strutil.c
#include "strutil.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

void str_to_lower(char *s) {
    if (!s) return;
    for (; *s; s++) {
        *s = (char)tolower((unsigned char)*s);
    }
}

void str_to_upper(char *s) {
    if (!s) return;
    for (; *s; s++) {
        *s = (char)toupper((unsigned char)*s);
    }
}

void str_capitalize(char *s) {
    if (!s || !*s) return;

    // Capitalize the first character
    *s = (char)toupper((unsigned char)*s);
    s++;

    // Lowercase the rest
    for (; *s; s++) {
        *s = (char)tolower((unsigned char)*s);
    }
}

char *str_parse_word(char *input, char *word) {
    if (!input) {
        word[0] = '\0';
        return NULL;
    }

    // Skip leading whitespace
    while (isspace((unsigned char)*input)) {
        ++input;
    }

    if (*input == '\0') {
        word[0] = '\0';
        return NULL;
    }

    // Copy word until next whitespace or null terminator
    char *out = word;
    while (*input && !isspace((unsigned char)*input)) {
        *out++ = *input++;
    }
    *out = '\0';

    // Skip trailing whitespace
    while (isspace((unsigned char)*input)) {
        ++input;
    }

    return *input ? input : NULL;
}
