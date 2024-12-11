#ifndef HADRON_READER_H
#define HADRON_READER_H 1

#define CHUNK_SIZE 1024

#include <cstdint>
#include <cstdio>

typedef enum __attribute__((__packed__)) FileMode {
  FILE_MODE_READ,
  FILE_MODE_WRITE,
} FileMode;

typedef enum __attribute__((__packed__)) FileResult {
  FILE_STATUS_OK,
  FILE_MODE_INVALID,
  FILE_READ_DONE,
  FILE_READ_FAILURE,
  FILE_WRITE_FAILURE,
} FileResult;

typedef struct FileHeader {
  char    magic[4];
  uint8_t major;
  uint8_t minor;
  uint8_t flags;
  uint8_t name;
} FileHeader;

typedef class File {
  uint8_t buffer[CHUNK_SIZE]{};

  FILE       *fp;
  const char *file_name;
  size_t      file_size{0};
  size_t      buffer_size{0};
  size_t      buffer_pos{0};
  size_t      position{0};

  bool       eof{false};
  FileMode   mode;
  FileResult write_flush();
  void       close();

  [[nodiscard]] uint8_t name_length() const;

  public:
   File(const char *file_name, FileMode mode);
  ~File() { close(); }

  void get_dir(char *path) const;
  void get_name(char *name) const;
  void get_ext(char *extension) const;

  FileResult read();
  FileResult read_byte(char *cp);
  FileResult lookup_byte(char *cp) const;
  FileResult current_byte(char *cp) const;
  FileResult read_chunk(char *buffer, size_t start, size_t length) const;

  template <typename T> FileResult write(T value);
  template <typename T> File      &operator<<(T value);

  [[nodiscard]] FileResult write_header() const;
  [[nodiscard]] FileResult read_header(FileHeader *header);
  [[nodiscard]] FileResult read_name(char *name, size_t length);

} File;

#endif // HADRON_READER_H
