// buffer.h
#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

typedef struct Buffer {
    char *data;
    size_t length;
    const size_t capacity;
    bool overflow;
} Buffer;

typedef struct DynamicBuffer {
    char *data;
    size_t length;
    size_t capacity;
} DynamicBuffer;

// Init/free for stack or embedded use
Buffer buffer_init(char *data, size_t capacity);

// Append raw data or strings
void buffer_append(Buffer *buf, const char *data, size_t size);
void buffer_append_str(Buffer *buf, const char *str);

// Clear content (length = 0), keep memory
void buffer_clear(Buffer *buf);

// Safe printf-style append to the buffer
void buffer_vappendf(Buffer *buf, const char *fmt, va_list args);
void buffer_appendf(Buffer *buf, const char *fmt, ...);

// Replaces buffer contents with formatted text.
void buffer_printf(Buffer *buf, const char *fmt, ...);

// Trims leading and trailing whitespace from the buffer
void buffer_trim(Buffer *buf);

// Init/free for stack or embedded use
void dbuffer_init(DynamicBuffer *buf, size_t initial_capacity);
void dbuffer_cleanup(DynamicBuffer *buf);

// Heap-allocated variant
DynamicBuffer *dbuffer_new(size_t initial_capacity);
void dbuffer_free(DynamicBuffer *buf);

// Append raw data or strings
void dbuffer_append(DynamicBuffer *buf, const char *data, size_t size);
void dbuffer_append_str(DynamicBuffer *buf, const char *str);

// Clear content (length = 0), keep memory
void dbuffer_clear(DynamicBuffer *buf);

// Ensure at least N bytes capacity
void dbuffer_reserve(DynamicBuffer *buf, size_t needed_capacity);

// Safe printf-style append to the buffer
void dbuffer_vappendf(DynamicBuffer *buf, const char *fmt, va_list args);
void dbuffer_appendf(DynamicBuffer *buf, const char *fmt, ...);

// Replaces buffer contents with formatted text.
void dbuffer_printf(DynamicBuffer *buf, const char *fmt, ...);

// Trims leading and trailing whitespace from the buffer
void dbuffer_trim(DynamicBuffer *buf);

#endif // BUFFER_H
