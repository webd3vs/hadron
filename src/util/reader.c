#include "reader.h"

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
}
