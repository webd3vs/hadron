#ifndef HADRON_BYTECODE_H
#define HADRON_BYTECODE_H 1

#include <cstdint>

typedef enum class Opcodes : char {
  OP_RETURN = 'r',
  OP_MOVE   = 'm', // prep for registers
  OP_ADD    = '+',
  OP_SUB    = '-',
  OP_MUL    = '*',
  OP_DIV    = '/',
  OP_NEGATE = 'n',
  OP_NOT    = '!',
  OP_B_NOT  = '~',
  OP_LOAD   = 'l',
  OP_STORE  = 's',
} Opcode;

#define MAX_INSTRUCTIONS 1024

typedef struct Chunk {
  int     pos{0};
  uint8_t code[MAX_INSTRUCTIONS]{};

  void write(Opcode opcode);
  void write(uint8_t value);
  void write(int8_t value);
  void write(char value);
  void write(int32_t value);
  void write(double value);
} Chunk;

#endif // HADRON_BYTECODE_H
