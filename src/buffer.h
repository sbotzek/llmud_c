#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct Buffer {
    char *data;
    size_t length;
    size_t capacity;
} Buffer;

// Init/free for stack or embedded use
bool buffer_init(Buffer *buf, size_t initial_capacity);
void buffer_free(Buffer *buf);

// Heap-allocated variant
Buffer *buffer_create(size_t initial_capacity);
void buffer_destroy(Buffer *buf);

// Append raw data or strings
bool buffer_append(Buffer *buf, const char *data, size_t size);
bool buffer_append_str(Buffer *buf, const char *str);

// Clear content (length = 0), keep memory
void buffer_clear(Buffer *buf);

// Ensure at least N bytes capacity
bool buffer_reserve(Buffer *buf, size_t needed_capacity);

// Safe printf-style append to the buffer
bool buffer_appendf(Buffer *buf, const char *fmt, ...);

#endif // BUFFER_H
