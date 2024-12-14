#ifndef HADRON_BYTECODE_H
#define HADRON_BYTECODE_H 1

#include <cstddef>
#include <cstdint>
#include <type_traits>

typedef enum class OpCodes : uint8_t {
  RETURN     = 'r',
  MOVE       = 'm',
  ADD        = '+',
  SUB        = '-',
  MUL        = '*',
  DIV        = '/',
  L_AND      = 'a',
  L_OR       = 'o',
  B_AND      = '&',
  B_OR       = '|',
  B_XOR      = '^',
  B_NOT      = '~',
  NOT        = '!',
  NEGATE     = 'n',
  LOAD       = 'l',
  STORE      = 's',
  RANGE_EXCL = 0x80,
  RANGE_L_IN = 0x81,
  RANGE_R_IN = 0x82,
  RANGE_INCL = 0x83,
  FX_ENTRY   = 0x90,
  FX_EXIT    = 0x91,
} OpCode;

#define MAX_INSTRUCTIONS 1024

typedef struct Chunk {
  int     pos{0};
  uint8_t code[MAX_INSTRUCTIONS]{};

  template <typename T> void write(T value) {
    if constexpr (std::is_same_v<T, char *>) {
      const size_t len = strlen(value);
      for (size_t i = 0; i < len; i++) {
        code[pos++] = value[i];
      }
    } else {
      *reinterpret_cast<T *>(code + pos) = value;
      pos += sizeof(T);
    }
  }
} Chunk;

#endif // HADRON_BYTECODE_H
