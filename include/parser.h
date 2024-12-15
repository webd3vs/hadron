#ifndef HADRON_PARSER_H
#define HADRON_PARSER_H 1

#include "lexer.h"
#include "logger.h"
#include "symbol.h"

typedef class Parser {
  public:
  Lexer      &lexer;
  Chunk      &chunk;
  Token       current_token{};
  Token       prev_token{};
  SymbolTable symbols;

  explicit Parser(Lexer &lexer, Chunk &chunk);
  void     advance();
  Token   &consume(Type type, const char *error);
  bool     match(Type type);
} Parser;

typedef void (*NudFn)(Parser &, const Token &);
typedef void (*LedFn)(Parser &, const Token &);

typedef enum class Precedence : int8_t {
  NUL = -1,
  LIT,
  ASG = 1,
  LOR,
  LND,
  BOR,
  XOR,
  BND,
  EQT,
  CMP,
  BSH,
  TRM,
  FCT,
  RNG,
  UNR,
  GRP,
  MAX,
} Precedence;

typedef struct ParseRule {
  Precedence precedence;
  NudFn      nud; // Null denotation function
  LedFn      led; // Left denotation function
} ParseRule;

void parse_expression(Parser &parser, Precedence precedence);

#endif // HADRON_PARSER_H
