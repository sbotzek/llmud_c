// strutil.h
#ifndef STRUTIL_H
#define STRUTIL_H

#include <stdbool.h>

// Returns a newly allocated copy of the input string.
// Aborts on allocation failure.
char *str_copy(const char *src);
bool str_token_contains(const char *list, const char *word);

// Parses a single word from the input into `word` buffer.
// Skips leading whitespace. If input is NULL or no word remains, sets word[0] = '\0' and returns NULL.
// Otherwise, copies the next space-delimited token into `word` and returns the new input position.
char *str_parse_word(char *input, char *word);

#endif // STRUTIL_H
