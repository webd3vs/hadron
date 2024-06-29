#include "reader.h"
#include <stdlib.h>

// todo return errors instead of exiting

Reader *newReader(const char *filename) {
  Reader *reader = malloc(sizeof(Reader));
  if (reader == NULL) {
    fprintf(stderr, "Could not allocate memory for reader");
    exit(EXIT_FAILURE);
  }
  reader->fp = fopen(filename, "rb");
  if (reader->fp == NULL) {
    fprintf(stderr, "Could not open file");
    free(reader);
    exit(EXIT_FAILURE);
  }
  return reader;
}

size_t readChunk(Reader *reader) {
  size_t read = fread(reader->buffer, 1, CHUNK_SIZE, reader->fp);
  if (read < CHUNK_SIZE) {
    if (feof(reader->fp)) {
      reader->buffer_size = read;
      return read;
    }
    fprintf(stderr, "Could not read file");
    exit(EXIT_FAILURE);
  }
  reader->buffer_size = CHUNK_SIZE;
  return read;
}
