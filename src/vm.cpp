#include "vm.hpp"
#include "bytecode.hpp"
#include "logger.hpp"

#include <cstdio>

InterpretResult VM::interpret(Chunk &chunk) {
  for (int ip = 0; ip < chunk.pos; ip++) {
    switch (const auto opcode = static_cast<OpCode>(chunk.code[ip]); opcode) {
      case OpCodes::FX_ENTRY:
      case OpCodes::FX_EXIT:
        break;
      case OpCodes::MOVE:
        stack[++sp] = *reinterpret_cast<double *>(chunk.code + ip + 2);
        ip += 9;
        break;
      case OpCodes::RETURN:
        printf("%g\n", stack[sp]);
        return INTERPRET_OK;
      case OpCodes::ADD:
        stack[sp - 1] = stack[sp - 1] + stack[sp];
        sp--;
        break;
      case OpCodes::MUL:
        stack[sp - 1] = stack[sp - 1] * stack[sp];
        sp--;
        break;
      case OpCodes::SUB:
        stack[sp - 1] = stack[sp - 1] - stack[sp];
        sp--;
        break;
      case OpCodes::DIV:
        stack[sp - 1] = stack[sp - 1] / stack[sp];
        sp--;
        break;
      case OpCodes::L_AND:
        stack[sp - 1] = stack[sp - 1] && stack[sp];
        sp--;
        break;
      case OpCodes::L_OR:
        stack[sp - 1] = stack[sp - 1] || stack[sp];
        sp--;
        break;
      case OpCodes::B_AND:
        // stack[sp - 1] = stack[sp - 1] & stack[sp];
        // sp--;
        Logger::fatal("& can only be applied to integers");
        break;
      case OpCodes::NEGATE:
        stack[sp] = -stack[sp];
        break;
      case OpCodes::NOT:
        stack[sp] = !static_cast<bool>(stack[sp]);
        break;
      case OpCodes::B_NOT:
        // stack[sp] = ~stack[sp];
        Logger::fatal("~ can only be applied to integers");
        break;
      case OpCodes::RANGE_EXCL:
      case OpCodes::RANGE_L_IN:
      case OpCodes::RANGE_R_IN:
      case OpCodes::RANGE_INCL:
        Logger::info("Range [%g, %g]", stack[sp - 1], stack[sp]);
        sp--;
        stack[sp] = 0;
        Logger::warn("Range operators not implemented yet");
        break;
      default:
        Logger::fatal("Unknown opcode\n");
    }
  }
  return INTERPRET_RUNTIME_ERROR;
}
