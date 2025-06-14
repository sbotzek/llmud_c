// buffer.c
#include "buffer.h"
#include "log.h"
#include "macros.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Internal struct for scratch buffers
typedef struct ScratchHeader {
    struct ScratchHeader *next;
    Buffer buffer;
} ScratchHeader;

static ScratchHeader *scratch_head = NULL;

void buffer_init(Buffer *buf, size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 1;
    }
    buf->data = malloc(initial_capacity);
    CHECK_MSG(buf->data != NULL, "buffer_init: malloc of %zu bytes failed", initial_capacity);
    buf->length = 0;
    buf->capacity = initial_capacity;
    buf->data[0] = '\0';
}

void buffer_cleanup(Buffer *buf) {
    if (!buf) return;
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->length = 0;
    buf->capacity = 0;
}

Buffer *buffer_new(size_t initial_capacity) {
    Buffer *buf = calloc(1, sizeof *buf);
    CHECK_MSG(buf != NULL,
              "buffer_new: calloc of %zu bytes failed", sizeof *buf);
    buffer_init(buf, initial_capacity);
    return buf;
}

void buffer_free(Buffer *buf) {
    if (!buf) return;
    buffer_cleanup(buf);
    free(buf);
}

Buffer *buffer_new_scratch(size_t initial_capacity) {
    ScratchHeader *hdr = calloc(1, sizeof(ScratchHeader));
    CHECK_MSG(hdr != NULL,
              "buffer_new_scratch: calloc of %zu bytes failed", sizeof *hdr);

    buffer_init(&hdr->buffer, initial_capacity);
    hdr->next = scratch_head;
    scratch_head = hdr;
    return &hdr->buffer;
}

void buffer_gc_scratch(void) {
    ScratchHeader *curr = scratch_head;
    while (curr) {
        ScratchHeader *next = curr->next;
        buffer_cleanup(&curr->buffer);
        free(curr);
        curr = next;
    }
    scratch_head = NULL;
}

void buffer_unscratch(Buffer *buf) {
    ScratchHeader *prev = NULL;
    ScratchHeader *curr = scratch_head;

    while (curr) {
        if (&curr->buffer == buf) {
            if (prev) {
                prev->next = curr->next;
            } else {
                scratch_head = curr->next;
            }

            // Allocate new heap buffer
            Buffer *heap_buf = malloc(sizeof(Buffer));
            CHECK_MSG(heap_buf != NULL, "buffer_unscratch: malloc failed");

            *heap_buf = *buf;  // copy contents
            buffer_cleanup(&curr->buffer); // no-op, but consistent
            free(curr);

            *buf = *heap_buf;
            free(heap_buf);
            return;
        }

        prev = curr;
        curr = curr->next;
    }

    CHECK_MSG(false, "buffer_unscratch: buffer not found in scratch list");
}

void buffer_printf(Buffer *buf, const char *fmt, ...) {
    buffer_clear(buf);

    va_list args;
    va_start(args, fmt);
    buffer_vappendf(buf, fmt, args);
    va_end(args);
}

void buffer_reserve(Buffer *buf, size_t needed_capacity) {
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

void buffer_append(Buffer *buf, const char *data, size_t size) {
    buffer_reserve(buf, buf->length + size + 1);
    memcpy(buf->data + buf->length, data, size);
    buf->length += size;
    buf->data[buf->length] = '\0';
}

void buffer_append_str(Buffer *buf, const char *str) {
    buffer_append(buf, str, strlen(str));
}

void buffer_clear(Buffer *buf) {
    if (!buf || !buf->data) return;
    buf->length = 0;
    buf->data[0] = '\0';
}

void buffer_vappendf(Buffer *buf, const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    char temp[512];
    int needed = vsnprintf(temp, sizeof(temp), fmt, args_copy);
    va_end(args_copy);
    CHECK_MSG(needed >= 0, "buffer_vappendf: vsnprintf error");
    if ((size_t)needed < sizeof(temp)) {
        buffer_append(buf, temp, (size_t)needed);
        return;
    }
    size_t size = (size_t)needed + 1;
    char *dynamic = malloc(size);
    CHECK_MSG(dynamic != NULL, "buffer_vappendf: malloc of %zu bytes failed", size);
    vsnprintf(dynamic, size, fmt, args);
    buffer_append(buf, dynamic, (size_t)needed);
    free(dynamic);
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
