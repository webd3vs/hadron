#ifndef HADRON_PARSER_H
#define HADRON_PARSER_H 1

#include "lexer.hpp"
#include "logger.hpp"

#include <symbol.hpp>

typedef struct Parser {
  Lexer      &lexer;
  Token       current_token{};
  Token       prev_token{};
  Chunk      &chunk;
  SymbolTable symbols;

  explicit Parser(Lexer &lexer, Chunk &chunk) : lexer(lexer), chunk(chunk) {
    advance();
  }
  void advance() {
    prev_token    = current_token;
    current_token = lexer.advance();
  }
  Token &consume(const Type type, const char *error) {
    if (current_token.type == type) {
      advance();
      return current_token;
    }
    Logger::fatal(error);
    return current_token; // never reached
  }
} Parser;

typedef void (*NudFn)(Parser &, const Token &);
typedef void (*LedFn)(Parser &, const Token &);

typedef enum class Precedence : int8_t {
  NUL = -1,
  ASG = 1,
  OR_,
  AND,
  EQT,
  CMP,
  TRM,
  FCT,
  UNR,
  GRP,
  LIT,
} Precedence;

typedef struct ParseRule {
  Precedence precedence;
  NudFn      nud; // Null denotation function
  LedFn      led; // Left denotation function
} ParseRule;

void parse_expression(Parser &parser, Precedence precedence);

#endif // HADRON_PARSER_H
