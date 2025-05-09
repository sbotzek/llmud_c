// buffer.h
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
void buffer_init(Buffer *buf, size_t initial_capacity);
void buffer_cleanup(Buffer *buf);

// Heap-allocated variant
Buffer *buffer_new(size_t initial_capacity);
void buffer_free(Buffer *buf);

// Allocates a new scratch buffer (auto-freed by buffer_gc_scratch)
Buffer *buffer_new_scratch(size_t initial_capacity);
// Frees all scratch buffers
void buffer_gc_scratch(void);
// Removes the buffer from scratch GC management.
// Crashes if buffer is not a scratch buffer.
void buffer_unscratch(Buffer *buf);

// Append raw data or strings
void buffer_append(Buffer *buf, const char *data, size_t size);
void buffer_append_str(Buffer *buf, const char *str);

// Clear content (length = 0), keep memory
void buffer_clear(Buffer *buf);

// Ensure at least N bytes capacity
void buffer_reserve(Buffer *buf, size_t needed_capacity);

// Safe printf-style append to the buffer
void buffer_vappendf(Buffer *buf, const char *fmt, va_list args);
void buffer_appendf(Buffer *buf, const char *fmt, ...);

// Replaces buffer contents with formatted text.
void buffer_printf(Buffer *buf, const char *fmt, ...);


// Trims leading and trailing whitespace from the buffer
void buffer_trim(Buffer *buf);

#endif // BUFFER_H
