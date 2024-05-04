#ifndef HADRON_READER_H
#define HADRON_READER_H

#define CHUNK_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

typedef struct Reader {
  FILE  *fp;
  char   buffer[CHUNK_SIZE];
  size_t buffer_size;
  size_t buffer_pos;
} Reader;

Reader *newReader(const char *filename);
void	freeReader(Reader *reader);
size_t	readChunk(Reader *reader);

#endif