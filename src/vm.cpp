#include "vm.hpp"
#include "bytecode.hpp"
#include "logger.hpp"

#include <cstdio>

InterpretResult VM::interpret(Chunk &chunk) {
  this->chunk = &chunk;

  for (int ip = 0; ip < chunk.pos; ip++) {
    switch (const auto opcode = static_cast<Opcode>(chunk.code[ip]); opcode) {
      case Opcodes::OP_MOVE:
        stack[++sp] = *reinterpret_cast<double *>(chunk.code + ip + 2);
        ip += 9;
        break;
      case Opcodes::OP_RETURN:
        printf("%g\n", stack[sp]);
        return INTERPRET_OK;
      case Opcodes::OP_ADD:
        stack[sp - 1] = stack[sp - 1] + stack[sp];
        sp--;
        break;
      case Opcodes::OP_MUL:
        stack[sp - 1] = stack[sp - 1] * stack[sp];
        sp--;
        break;
      case Opcodes::OP_SUB:
        stack[sp - 1] = stack[sp - 1] - stack[sp];
        sp--;
        break;
      case Opcodes::OP_DIV:
        stack[sp - 1] = stack[sp - 1] / stack[sp];
        sp--;
        break;
      case Opcodes::OP_NEGATE:
        stack[sp] = -stack[sp];
        break;
      case Opcodes::OP_NOT:
        stack[sp] = !stack[sp];
        break;
      case Opcodes::OP_B_NOT:
        // stack[sp] = ~stack[sp];
        Logger::fatal("~ can only be applied to integers");
        break;
      default:
        Logger::fatal("Unknown opcode\n");
    }
  }
  return INTERPRET_RUNTIME_ERROR;
}
