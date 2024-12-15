#ifndef HADRON_VM_H
#define HADRON_VM_H

#include "bytecode.h"

typedef enum InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

#define MAX_STACK     0x100
#define MAX_CONSTANTS 0x100

typedef class VM {
  // Chunk *chunk;
  // uint8_t *ip;
  double stack[MAX_STACK]{};
  // double constants[MAX_CONSTANTS]{};
  int sp{-1};
  // int pc{-1};

  public:
  VM() = default;

  InterpretResult interpret(Chunk &chunk);
} VM;

#endif // HADRON_VM_H
