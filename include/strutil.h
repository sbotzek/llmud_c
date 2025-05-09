// strutil.h
#ifndef STRUTIL_H
#define STRUTIL_H

#include <stdbool.h>

// Returns a newly allocated copy of the input string.
// Aborts on allocation failure.
char *str_copy(const char *src);
bool str_token_contains(const char *list, const char *word);

#endif // STRUTIL_H
