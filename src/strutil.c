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
