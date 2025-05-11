// file_chunk.h

/* Helps with reading/writing files of the standard format:

For when you have multiples in a file:
<example1>
#room
id: 1023~
name: a very dark room~
description:
You stand in a very dark room.  Please
don't be afraid~
#exit north 1024~
#exit south 2048~
#exit
direction: east~
to_room: 4096~
description:
You see a very far dropoff that direction.  You
think it would be a bad idea to go that way.~
#end exit
#end room
#room
id: 1024~
name: a very bright room~
description:
You stand in a very bright room.~
#exit south 1023~
#end room
</example1>

Or, if you don't have multiples
<example2>
username: bobby~
password_hash: jlkr32~
#character ricky~
#character junior~
</example2>
*/
#ifndef FILE_CHUNK_H
#define FILE_CHUNK_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "buffer.h"

typedef enum {
    FILE_CHUNK_FIELD,
    FILE_CHUNK_SECTION_START,
    FILE_CHUNK_SECTION_END
} FileChunkType;

typedef struct FileChunk {
    FileChunkType type;
    Buffer tag;    // For FIELD: key; for SECTION: section name
    Buffer value;  // Only valid for FIELD
} FileChunk;

typedef struct FileChunkReader {
    FILE   *fp;
    int     line_number;

    Buffer  raw;    // Raw chunk line, including trailing ~
    FileChunk chunk;
} FileChunkReader;

void file_chunk_reader_init(FileChunkReader *r, FILE *fp);
void file_chunk_reader_cleanup(FileChunkReader *r);

// Returns true if another chunk was read, false on EOF or error
bool file_chunk_read(FileChunkReader *r);
// Skips until the end of the given section is found.
// Returns true if #end <section> found successfully, false on EOF.
bool file_chunk_skip_section(FileChunkReader *r, const char *section);

// writes a field of the format:
// <tag>: <value>~
void file_chunk_write_field(FILE *fp, const char *tag, const char *value);
// writes an inline section:
// #<section> <args>~
void file_chunk_write_section_inline(FILE *fp, const char *section, const char *args);
// writes the start of a block section:
// #<section>
void file_chunk_write_section_start(FILE *fp, const char *section);
// writes the end of a block section:
// #end <section>
void file_chunk_write_section_end(FILE *fp, const char *section);


#endif // FILE_CHUNK_H
