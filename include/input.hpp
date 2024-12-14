#ifndef HADRON_INPUT_H
#define HADRON_INPUT_H 1

#include "file.hpp"

#include <cstring>

class InputSource {
  public:
           InputSource() = default;
  virtual ~InputSource();

  virtual char               next()                                = 0;
  [[nodiscard]] virtual char peek() const                          = 0;
  [[nodiscard]] virtual char current() const                       = 0;
  virtual void read_chunk(char *dest, size_t start, size_t length) = 0;
};

class FileInput final : public InputSource {
  File &file;
  bool  end{false};
  char  current_char{'\0'};

  public:
  explicit FileInput(File &file) : file(file) {}

  char               next() override;
  [[nodiscard]] char peek() const override;
  [[nodiscard]] char current() const override;
  void read_chunk(char *dest, size_t start, size_t length) override;
};

class StringInput final : public InputSource {
  const char *source;
  size_t      index{0};
  size_t      length{0};
  char        current_char{'\0'};

  public:
  explicit StringInput(const char *source)
    : source(source), length(strnlen(source, 0x400)) {}

  char               next() override;
  [[nodiscard]] char peek() const override;
  [[nodiscard]] char current() const override;
  void read_chunk(char *dest, size_t start, size_t length) override;
};

#endif // HADRON_INPUT_H
