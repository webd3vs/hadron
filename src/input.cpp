#include "input.hpp"

InputSource::~InputSource() = default;

char FileInput::next() {
  char c;
  if (file.read_byte(&c) == FILE_READ_FAILURE) {
    printf("File read failure\n");
    end          = true;
    current_char = '\0';
  }
  return current_char = c;
}

char FileInput::peek() const {
  char c;
  if (file.lookup_byte(&c) == FILE_STATUS_OK) {
    return c;
  }
  return '\0';
}

char FileInput::current() const { return current_char; }

void FileInput::read_chunk(
  char *dest, const size_t start, const size_t length) {
  file.read_chunk(dest, start, length);
}

char StringInput::next() {
  if (index >= length) {
    return '\0';
  }
  return current_char = source[index++];
}

char StringInput::peek() const {
  if (index >= length) {
    return '\0';
  }
  return source[index];
}

char StringInput::current() const { return current_char; }

void StringInput::read_chunk(
  char *dest, const size_t start, const size_t length) {
  strncpy(dest, source + start, length);
  dest[length] = '\0';
}
