// buffer.c
#include "buffer.h"
#include "log.h"
#include "macros.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

void buffer_init(Buffer *buf, char *data, size_t capacity) {
    buf->data = data;
    buf->length = 0;
    *((size_t*)&buf->capacity) = capacity;
    buf->data[0] = '\0';
}

void buffer_printf(Buffer *buf, const char *fmt, ...) {
    buffer_clear(buf);

    va_list args;
    va_start(args, fmt);
    buffer_vappendf(buf, fmt, args);
    va_end(args);
}

void buffer_append(Buffer *buf, const char *data, size_t size) {
    CHECK(buf->length < buf->capacity);

    if (buf->overflow) {
        return;
    }

    if (size >= buf->capacity - buf->length) {
        log_warn("append size %zu >= remaining capacity %zu", size, buf->capacity - buf->length);
        size = buf->capacity - buf->length - 1;
        buf->overflow = true;
    }

    memcpy(buf->data + buf->length, data, size);
    buf->length += size;
    buf->data[buf->length] = '\0';
}

void buffer_append_str(Buffer *buf, const char *str) {
    buffer_append(buf, str, strlen(str));
}

void buffer_clear(Buffer *buf) {
    buf->length = 0;
    buf->data[0] = '\0';
    buf->overflow = false;
}

void buffer_vappendf(Buffer *buf, const char *fmt, va_list args) {
    size_t remaining = (buf->capacity > buf->length) ? (buf->capacity - buf->length) : 0;

    if (remaining <= 1) {
        buf->overflow = true;
        return;
    }

    va_list args_copy;
    va_copy(args_copy, args);

    char temp[remaining];  // remaining ≥ 2, safe for vsnprintf + \0
    int needed = vsnprintf(temp, remaining, fmt, args_copy);

    va_end(args_copy);

    if (needed < 0) {
        log_warn("buffer_vappendf: vsnprintf error");
        buf->overflow = true;
        return;
    }

    // Write whatever vsnprintf gave us — it is always null-terminated
    buffer_append(buf, temp, strlen(temp));

    if ((size_t)needed >= remaining) {
        // vsnprintf result was truncated — it needed more room
        buf->overflow = true;
    }
}


void buffer_appendf(Buffer *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    buffer_vappendf(buf, fmt, args);
    va_end(args);
}

void buffer_trim(Buffer *buf) {
    if (!buf || buf->length == 0) return;
    size_t start = 0;
    size_t end = buf->length;
    while (start < end && isspace((unsigned char)buf->data[start])) start++;
    while (end > start && isspace((unsigned char)buf->data[end - 1])) end--;
    size_t new_len = end - start;
    if (start > 0 && new_len > 0) {
        memmove(buf->data, buf->data + start, new_len);
    }
    buf->data[new_len] = '\0';
    buf->length = new_len;
}

void dbuffer_init(DynamicBuffer *buf, size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 1;
    }
    buf->data = malloc(initial_capacity);
    CHECK_MSG(buf->data != NULL, "buffer_init: malloc of %zu bytes failed", initial_capacity);
    buf->length = 0;
    buf->capacity = initial_capacity;
    buf->data[0] = '\0';
}

void dbuffer_cleanup(DynamicBuffer *buf) {
    if (!buf) return;
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->length = 0;
    buf->capacity = 0;
}

DynamicBuffer *dbuffer_new(size_t initial_capacity) {
    DynamicBuffer *buf = calloc(1, sizeof *buf);
    CHECK_MSG(buf != NULL,
              "buffer_new: calloc of %zu bytes failed", sizeof *buf);
    dbuffer_init(buf, initial_capacity);
    return buf;
}

void dbuffer_free(DynamicBuffer *buf) {
    if (!buf) return;
    dbuffer_cleanup(buf);
    free(buf);
}

void dbuffer_printf(DynamicBuffer *buf, const char *fmt, ...) {
    dbuffer_clear(buf);

    va_list args;
    va_start(args, fmt);
    dbuffer_vappendf(buf, fmt, args);
    va_end(args);
}

void dbuffer_reserve(DynamicBuffer *buf, size_t needed_capacity) {
    if (needed_capacity <= buf->capacity) return;
    size_t new_capacity = buf->capacity ? buf->capacity : 64;
    while (new_capacity < needed_capacity) {
        new_capacity *= 2;
    }
    char *new_data = realloc(buf->data, new_capacity);
    CHECK_MSG(new_data != NULL, "buffer_reserve: realloc to %zu bytes failed", new_capacity);
    buf->data = new_data;
    buf->capacity = new_capacity;
}

void dbuffer_append(DynamicBuffer *buf, const char *data, size_t size) {
    dbuffer_reserve(buf, buf->length + size + 1);
    memcpy(buf->data + buf->length, data, size);
    buf->length += size;
    buf->data[buf->length] = '\0';
}

void dbuffer_append_str(DynamicBuffer *buf, const char *str) {
    dbuffer_append(buf, str, strlen(str));
}

void dbuffer_clear(DynamicBuffer *buf) {
    if (!buf || !buf->data) return;
    buf->length = 0;
    buf->data[0] = '\0';
}

void dbuffer_vappendf(DynamicBuffer *buf, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    char temp[512];
    int needed = vsnprintf(temp, sizeof(temp), fmt, args_copy);
    va_end(args_copy);
    CHECK_MSG(needed >= 0, "buffer_vappendf: vsnprintf error");
    if ((size_t)needed < sizeof(temp)) {
        dbuffer_append(buf, temp, (size_t)needed);
        return;
    }
    size_t size = (size_t)needed + 1;
    char *dynamic = malloc(size);
    CHECK_MSG(dynamic != NULL, "buffer_vappendf: malloc of %zu bytes failed", size);
    vsnprintf(dynamic, size, fmt, args);
    dbuffer_append(buf, dynamic, (size_t)needed);
    free(dynamic);
}

void dbuffer_appendf(DynamicBuffer *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dbuffer_vappendf(buf, fmt, args);
    va_end(args);
}

void dbuffer_trim(DynamicBuffer *buf) {
    if (!buf || buf->length == 0) return;
    size_t start = 0;
    size_t end = buf->length;
    while (start < end && isspace((unsigned char)buf->data[start])) start++;
    while (end > start && isspace((unsigned char)buf->data[end - 1])) end--;
    size_t new_len = end - start;
    if (start > 0 && new_len > 0) {
        memmove(buf->data, buf->data + start, new_len);
    }
    buf->data[new_len] = '\0';
    buf->length = new_len;
}


