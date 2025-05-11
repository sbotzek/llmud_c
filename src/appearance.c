// appearance.c

#include <stdlib.h>
#include <string.h>

#include "appearance.h"
#include "macros.h"
#include "log.h"
#include "strutil.h"
#include "file_chunk.h"

// static functions
static void read_field(Appearance *a, const FileChunk *chunk);

// appearance_init
void appearance_init(Appearance *a) {
    a->name = NULL;
    a->long_name = NULL;
    a->description = NULL;
}

// appearance_cleanup
void appearance_cleanup(Appearance *a) {
    free(a->name);
    free(a->description);
    a->name = NULL;
    a->description = NULL;
}

// appearance_new
Appearance *appearance_new(void) {
    Appearance *a = malloc(sizeof(Appearance));
    CHECK_MSG(a != NULL, "Out of memory allocating Appearance");
    appearance_init(a);
    return a;
}

// appearance_free
void appearance_free(Appearance *a) {
    if (!a) return;
    appearance_cleanup(a);
    free(a);
}

// appearance_read_section
void appearance_read_section(Appearance *a, FileChunkReader *r, const char *section) {
    CHECK(a != NULL);
    CHECK(r != NULL);

    while (file_chunk_read(r)) {
        FileChunk *chunk = &r->chunk;

        if (chunk->type == FILE_CHUNK_SECTION_END &&
            strcmp(chunk->tag.data, section) == 0) {
            return;
        }

        if (chunk->type != FILE_CHUNK_FIELD) {
            log_warn("Skipping unexpected chunk type in appearance at line %d", r->line_number);
            continue;
        }

        read_field(a, chunk);
    }

    log_warn("Unterminated #appearance section");
}

// appearance_write_section
void appearance_write_section(const Appearance *a, FILE *fp, const char *section) {
    CHECK(a != NULL);
    CHECK(fp != NULL);

    file_chunk_write_section_start(fp, section);

    if (a->name) {
        file_chunk_write_field(fp, "name", a->name);
    }

    if (a->long_name) {
        file_chunk_write_field(fp, "long_name", a->description);
    }

    if (a->description) {
        file_chunk_write_field(fp, "description", a->description);
    }

    file_chunk_write_section_end(fp, section);
}

// read_field
static void read_field(Appearance *a, const FileChunk *chunk) {
    const char *tag = chunk->tag.data;
    const char *val = chunk->value.data;

    if (strcmp(tag, "name") == 0) {
        free(a->name);
        a->name = str_copy(val);
    } else if (strcmp(tag, "long_name") == 0) {
        free(a->long_name);
        a->long_name = str_copy(val);
    } else if (strcmp(tag, "description") == 0) {
        free(a->description);
        a->description = str_copy(val);
    } else {
        log_warn("Unknown field '%s' in appearance", tag);
    }
}
