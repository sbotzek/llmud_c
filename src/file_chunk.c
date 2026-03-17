// file_chunk.c

#include "file_chunk.h"
#include "macros.h"
#include "log.h"

#include <string.h>
#include <ctype.h>

// Static function declarations
static void parse_chunk(FileChunkReader *r);
static void chunk_reset(FileChunk *chunk);
static void trim_trailing_newlines(DynamicBuffer *buf);

// Implementation

void file_chunk_reader_init(FileChunkReader *r, FILE *fp) {
    CHECK(r);
    CHECK(fp);

    r->fp = fp;
    r->line_number = 0;

    dbuffer_init(&r->raw, 128);
    dbuffer_init(&r->chunk.tag, 32);
    dbuffer_init(&r->chunk.value, 64);
    r->chunk.type = FILE_CHUNK_FIELD;
}

void file_chunk_reader_cleanup(FileChunkReader *r) {
    CHECK(r);
    dbuffer_cleanup(&r->raw);
    dbuffer_cleanup(&r->chunk.tag);
    dbuffer_cleanup(&r->chunk.value);
}

bool file_chunk_read(FileChunkReader *r) {
    CHECK(r);

    dbuffer_clear(&r->raw);
    chunk_reset(&r->chunk);

    int first_ch;
    int ch;
    while ((ch = fgetc(r->fp)) != EOF) {
        r->line_number++;

        first_ch = ch;
        while (ch != '~'
            && (first_ch != '#' || (ch != '\r' && ch != '\n'))
            && ch != EOF) {
            dbuffer_append(&r->raw, (char *)&ch, 1);
            ch = fgetc(r->fp);
        }

        if (ch == EOF && strncmp(r->raw.data, "#end ", 5) != 0) {
            // Unexpected EOF without tilde or end section
            log_warn("Unexpected eof");
            return false;
        }

        // consume newline if present after tilde
        int next = fgetc(r->fp);
        if (next != '\n' && next != '\r' && next != EOF) {
            ungetc(next, r->fp);
        }

        // Got a full raw line, parse it
        trim_trailing_newlines(&r->raw);
        parse_chunk(r);

        log_debug("Got chunk tag %s, data %s", r->chunk.tag.data, r->chunk.value.data);
        return true;
    }

    return false;
}

// Internal helpers

static void chunk_reset(FileChunk *chunk) {
    dbuffer_clear(&chunk->tag);
    dbuffer_clear(&chunk->value);
    chunk->type = FILE_CHUNK_FIELD;
}

static void trim_trailing_newlines(DynamicBuffer *buf) {
    while (buf->length > 0 &&
          (buf->data[buf->length - 1] == '\n' ||
           buf->data[buf->length - 1] == '\r')) {
        buf->length--;
    }

    if (buf->data) {
        buf->data[buf->length] = '\0';
    }
}

static void parse_chunk(FileChunkReader *r) {
    const char *line = r->raw.data;

    // SECTION START or END
    if (line[0] == '#') {
        const char *rest = line + 1;
        const char *space = strchr(rest, ' ');

        if (space) {
            size_t tag_len = space - rest;
            dbuffer_append(&r->chunk.tag, rest, tag_len);
            dbuffer_append_str(&r->chunk.value, space + 1);
        } else {
            dbuffer_append_str(&r->chunk.tag, rest);
            dbuffer_clear(&r->chunk.value);
        }

        dbuffer_trim(&r->chunk.tag);
        dbuffer_trim(&r->chunk.value);

        if (strcmp(r->chunk.tag.data, "end") == 0) {
            // Move the actual section name into tag
            dbuffer_clear(&r->chunk.tag);
            dbuffer_append_str(&r->chunk.tag, r->chunk.value.data);
            dbuffer_trim(&r->chunk.tag);
            dbuffer_clear(&r->chunk.value);
            r->chunk.type = FILE_CHUNK_SECTION_END;
        } else {
            r->chunk.type = FILE_CHUNK_SECTION_START;
        }

        return;
    }

    // FIELD like "name: foo~"
    const char *sep = strchr(line, ':');
    CHECK_MSG(sep, "Expected colon in field line at line %d", r->line_number);

    size_t key_len = sep - line;
    dbuffer_append(&r->chunk.tag, line, key_len);
    dbuffer_trim(&r->chunk.tag);

    const char *val = sep + 1;
    dbuffer_append_str(&r->chunk.value, val);
    dbuffer_trim(&r->chunk.value);

    r->chunk.type = FILE_CHUNK_FIELD;
}

bool file_chunk_skip_section(FileChunkReader *r, const char *section) {
    CHECK(r != NULL);
    CHECK(section != NULL);

    while (file_chunk_read(r)) {
        if (r->chunk.type == FILE_CHUNK_SECTION_END &&
            strcmp(r->chunk.tag.data, section) == 0) {
            return true;
            }
    }

    log_warn("file_chunk_skip_section: Unterminated section #%s", section);
    return false;
}

void file_chunk_write_field(FILE *fp, const char *tag, const char *value) {
    CHECK(fp);
    CHECK(tag);
    CHECK(value);

    int written = fprintf(fp, "%s:%s~\n", tag, value);
    CHECK_MSG(written >= 0, "Failed to write field '%s'", tag);
}

void file_chunk_write_int_field(FILE *fp, const char *tag, int value) {
    CHECK(fp);
    CHECK(tag);

    int written = fprintf(fp, "%s:%d~\n", tag, value);
    CHECK_MSG(written >= 0, "Failed to write field '%s'", tag);
}

void file_chunk_write_section_inline(FILE *fp, const char *section, const char *args) {
    CHECK(fp);
    CHECK(section);
    CHECK(args && *args);

    int written = fprintf(fp, "#%s %s~\n", section, args);
    CHECK_MSG(written >= 0, "Failed to write inline section: #%s %s~", section, args);
}

void file_chunk_write_section_start(FILE *fp, const char *section) {
    CHECK(fp);
    CHECK(section);

    int written = fprintf(fp, "#%s\n", section);
    CHECK_MSG(written >= 0, "Failed to write section start: #%s", section);
}

void file_chunk_write_section_end(FILE *fp, const char *section) {
    CHECK(fp);
    CHECK(section);

    int written = fprintf(fp, "#end %s\n", section);
    CHECK_MSG(written >= 0, "Failed to write section end: #end %s", section);
}
