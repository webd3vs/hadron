#include "logger.hpp"
#include "types.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

static const char *getData(const Token &t) {
  switch (t.type) {
    case Types::CMP_EQ:
      return "==";
    case Types::CMP_NEQ:
      return "!=";
    case Types::CMP_GT:
      return ">";
    case Types::CMP_GEQ:
      return ">=";
    case Types::CMP_LT:
      return "<";
    case Types::CMP_LEQ:
      return "<=";
    case Types::CST_EQ:
      return "#=";
    case Types::SET_EQ:
      return "$=";
    case Types::EQ:
      return "=";
    case Types::ADD_EQ:
      return "+=";
    case Types::SUB_EQ:
      return "-=";
    case Types::MUL_EQ:
      return "*=";
    case Types::DIV_EQ:
      return "/=";
    case Types::INCR:
      return "++";
    case Types::DECR:
      return "--";
    case Types::L_AND_EQ:
      return "&&=";
    case Types::L_OR_EQ:
      return "||=";
    case Types::B_AND_EQ:
      return "&=";
    case Types::B_OR_EQ:
      return "|=";
    case Types::B_XOR_EQ:
      return "^=";
    case Types::POW_EQ:
      return "**=";
    case Types::REM_EQ:
      return "%=";
    case Types::R_SHIFT_EQ:
      return ">>=";
    case Types::L_SHIFT_EQ:
      return "<<=";
    case Types::AS:
      return "as";
    case Types::ASYNC:
      return "async";
    case Types::AWAIT:
      return "await";
    case Types::CASE:
      return "case";
    case Types::CLASS:
      return "class";
    case Types::DEFAULT:
      return "default";
    case Types::DO:
      return "do";
    case Types::ELSE:
      return "else";
    case Types::FALSE:
      return "false";
    case Types::FOR:
      return "for";
    case Types::FROM:
      return "from";
    case Types::FX:
      return "fx";
    case Types::IF:
      return "if";
    case Types::IMPORT:
      return "import";
    case Types::NEW:
      return "new";
    case Types::RETURN:
      return "return";
    case Types::SELECT:
      return "select";
    case Types::SWITCH:
      return "switch";
    case Types::TRUE:
      return "true";
    case Types::WHILE:
      return "while";
    case Types::NUL:
      return "null";
    case Types::STR:
      return "STR";
    case Types::NAME:
      return "NAME";
    case Types::DEC:
      using namespace std;
      return (string("NUM ") + to_string(t.value.f64)).c_str();
    case Types::DOT:
      return ".";
    case Types::RANGE_EXCL:
      return "..";
    case Types::RANGE_R_IN:
      return "..=";
    case Types::RANGE_L_IN:
      return "=..";
    case Types::RANGE_INCL:
      return "=..=";
    case Types::SEMICOLON:
      return ";";
    case Types::NEWLINE:
      return "\\n";
    case Types::COMMA:
      return ",";
    case Types::AT:
      return "@";
    case Types::HASH:
      return "#";
    case Types::DOLLAR:
      return "$";
    case Types::QUERY:
      return "?";
    case Types::L_BRACKET:
      return "[";
    case Types::R_BRACKET:
      return "]";
    case Types::L_PAREN:
      return "(";
    case Types::R_PAREN:
      return ")";
    case Types::L_CURLY:
      return "{";
    case Types::R_CURLY:
      return "}";
    case Types::HEX:
      return "HEX";
    case Types::OCTAL:
      return "OCT";
    case Types::BINARY:
      return "BIN";
    case Types::COLON:
      return ":";
    case Types::END:
      return "\\0";
    case Types::ADD:
      return "+";
    case Types::SUB:
      return "-";
    case Types::MUL:
      return "*";
    case Types::DIV:
      return "/";
    case Types::L_AND:
      return "&&";
    case Types::L_OR:
      return "||";
    case Types::B_AND:
      return "&";
    case Types::B_OR:
      return "|";
    case Types::CARET:
      return "^";
    case Types::B_NOT:
      return "~";
    case Types::L_NOT:
      return "!";
    case Types::L_SHIFT:
      return "<<";
    case Types::R_SHIFT:
      return ">>";
    case Types::POW:
      return "**";
    case Types::REM:
      return "%";
    case Types::MAX_TOKENS:
      return "MAX";
    case Types::ERROR:
      return "ERROR";
  }
  return "";
}

void Logger::info(const char *msg) { printf("\x1b[96mINFO\x1b[m %s\n", msg); }

void Logger::warn(const char *msg) { printf("\x1b[93mWARN\x1b[m %s\n", msg); }

void Logger::error(const char *msg) { printf("\x1b[91mERROR\x1b[m %s\n", msg); }

void Logger::fatal(const char *msg) {
  printf("\x1b[91;7;1m FATAL \x1b[0;91m %s\x1b[m\n", msg);
  exit(EXIT_FAILURE);
}

void Logger::print_token(const Token &token) {
  const auto key0   = "\x1b[94m";
  const auto key1   = "\x1b[96m";
  const auto text   = "\x1b[92m";
  const auto value  = "\x1b[93m";
  const auto clear  = "\x1b[m";
  const auto italic = "\x1b[3m";
  printf("%s\x1b[37mToken%s ", italic, clear);
  printf("<%s%s%s> ", text, getData(token), clear);
  printf("{ %stype%s: ", key0, clear);
  printf("%s%i%s, %spos%s: { %sline%s: ", value,
    static_cast<uint8_t>(token.type), clear, key0, clear, key1, clear);
  printf("%s%i%s, %sstart%s: ", value, token.pos.line, clear, key1, clear);
  printf("%s%i%s, %send%s: ", value, token.pos.start, clear, key1, clear);
  printf("%s%i%s, %sabsStart%s: ", value, token.pos.end, clear, key1, clear);
  printf("%s%i%s, %sabsEnd%s: ", value, token.pos.absStart, clear, key1, clear);
  printf("%s%i%s }\n", value, token.pos.absEnd, clear);
}

inline void print_bytes(
  const int bytes, const Chunk &chunk, int *offset, const char *desc) {
  for (int i = 0; i < bytes; i++) {
    printf("%02x ", chunk.code[(*offset)++]);
  }
  for (int i = bytes; i < 10; i++) {
    printf("   ");
  }
  printf("%s\n", desc);
}

void Logger::disassemble(const Chunk &chunk, const char *name) {
  printf("=== %s ===\n", name);

  for (int offset = 0; offset < chunk.pos;) {
    printf(" %04x: ", offset);

    switch (const auto instr = static_cast<Opcode>(chunk.code[offset])) {
      case Opcodes::OP_RETURN:
        print_bytes(1, chunk, &offset, "RET");
        break;
      case Opcodes::OP_ADD:
        print_bytes(1, chunk, &offset, "ADD");
        break;
      case Opcodes::OP_SUB:
        print_bytes(1, chunk, &offset, "SUB");
        break;
      case Opcodes::OP_MUL:
        print_bytes(1, chunk, &offset, "MUL");
        break;
      case Opcodes::OP_DIV:
        print_bytes(1, chunk, &offset, "DIV");
        break;
      case Opcodes::OP_NEGATE:
        print_bytes(1, chunk, &offset, "NEG");
        break;
      case Opcodes::OP_NOT:
        print_bytes(1, chunk, &offset, "NOT");
        break;
      case Opcodes::OP_B_NOT:
        print_bytes(1, chunk, &offset, "B_NOT");
        break;
      case Opcodes::OP_MOVE:
        print_bytes(10, chunk, &offset, "MOVE");
        break;
      case Opcodes::OP_LOAD:
        print_bytes(2, chunk, &offset, "LOAD");
        break;
      case Opcodes::OP_STORE:
        print_bytes(2, chunk, &offset, "STORE");
        break;
      default:
        printf("Unknown opcode %d\n", static_cast<char>(instr));
        ++offset;
    }
  }
}
