#include "bytecode.hpp"

#include <cstdlib>

void Chunk::write(Opcode opcode) { code[pos++] = static_cast<uint8_t>(opcode); }
void Chunk::write(const uint8_t value) { code[pos++] = value; }
void Chunk::write(const int8_t value) { code[pos++] = value; }
void Chunk::write(const char value) { code[pos++] = value; }
void Chunk::write(const int32_t value) {
  code[pos] = value;
  pos += sizeof(int32_t);
}
void Chunk::write(const double value) {
  *reinterpret_cast<double *>(code + pos) = value;
  pos += sizeof(double);
}
