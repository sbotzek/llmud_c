#include "buffer.h"
#include "log.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

bool buffer_init(Buffer *buf, size_t initial_capacity) {
    buf->data = malloc(initial_capacity);
    if (!buf->data) return false;

    buf->length = 0;
    buf->capacity = initial_capacity;
    return true;
}

void buffer_free(Buffer *buf) {
    if (buf && buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    if (buf) {
        buf->length = 0;
        buf->capacity = 0;
    }
}

Buffer *buffer_create(size_t initial_capacity) {
    Buffer *buf = malloc(sizeof(Buffer));
    if (!buf) return NULL;
    if (!buffer_init(buf, initial_capacity)) {
        free(buf);
        return NULL;
    }
    return buf;
}

void buffer_destroy(Buffer *buf) {
    if (!buf) return;
    buffer_free(buf);
    free(buf);
}

bool buffer_reserve(Buffer *buf, size_t needed_capacity) {
    if (needed_capacity <= buf->capacity) return true;

    size_t new_capacity = buf->capacity ? buf->capacity : 64;
    while (new_capacity < needed_capacity) {
        new_capacity *= 2;
    }

    char *new_data = realloc(buf->data, new_capacity);
    if (!new_data) return false;

    buf->data = new_data;
    buf->capacity = new_capacity;
    return true;
}

bool buffer_append(Buffer *buf, const char *data, size_t size) {
    if (!buffer_reserve(buf, buf->length + size + 1)) return false;

    memcpy(buf->data + buf->length, data, size);
    buf->length += size;
    buf->data[buf->length] = '\0'; // always null-terminate

    return true;
}

bool buffer_append_str(Buffer *buf, const char *str) {
    return buffer_append(buf, str, strlen(str));
}

void buffer_clear(Buffer *buf) {
    if (buf && buf->data) buf->data[0] = '\0';
    if (buf) buf->length = 0;
}

bool buffer_vappendf(Buffer *buf, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    char temp[512];
    int needed = vsnprintf(temp, sizeof(temp), fmt, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        return false;
    }

    if ((size_t)needed < sizeof(temp)) {
        return buffer_append(buf, temp, (size_t)needed);
    }

    // Fallback to heap allocation if it doesn't fit in the temp buffer
    size_t size = (size_t)needed + 1;
    char *dynamic = malloc(size);
    if (!dynamic) return false;

    vsnprintf(dynamic, size, fmt, args);
    bool success = buffer_append(buf, dynamic, (size_t)needed);
    free(dynamic);
    return success;
}

bool buffer_appendf(Buffer *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    bool result = buffer_vappendf(buf, fmt, args);
    va_end(args);
    return result;
}

void buffer_trim(Buffer *buf) {
    if (!buf || buf->length == 0) return;

    size_t start = 0;
    size_t end = buf->length;

    // Trim leading
    while (start < end && isspace((unsigned char)buf->data[start])) {
        start++;
    }

    // Trim trailing
    while (end > start && isspace((unsigned char)buf->data[end - 1])) {
        end--;
    }

    size_t new_len = end - start;

    if (start > 0 && new_len > 0) {
        memmove(buf->data, buf->data + start, new_len);
    }

    buf->data[new_len] = '\0';
    buf->length = new_len;
}

