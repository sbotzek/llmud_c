// appearance.h
#ifndef APPEARANCE_H
#define APPEARANCE_H

#include <stdbool.h>
#include <stdio.h>

typedef struct FileChunkReader FileChunkReader;

typedef struct Appearance {
    char *name;
    char *long_name;
    char *description;
} Appearance;

// Init/free for stack or embedded use
void appearance_init(Appearance *a);
void appearance_cleanup(Appearance *a);

// Heap-allocated variant
Appearance *appearance_new(void);
void        appearance_free(Appearance *a);

// Read/write section format, excluding the #appearance / #end appearance headers
// Caller must handle section dispatch and boundaries
void appearance_read_section(Appearance *a, FileChunkReader *r);
void appearance_write_section(const Appearance *a, FILE *fp);

#endif // APPEARANCE_H
