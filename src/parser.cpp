#include "parser.hpp"
#include "bytecode.hpp"
#include "types.hpp"

ParseRule &get_rule(Type token_type);

#define DEBUG false

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static NudFn parse_lit = [](Parser &parser, const Token &token) {
  if constexpr (DEBUG)
    printf("==> literal\n");
  // Logger::print_token(token);
  parser.chunk.write(Opcodes::OP_MOVE);
  parser.chunk.write(token.index);
  parser.chunk.write(token.value.f64);
  if constexpr (DEBUG)
    printf("<== literal\n");
};

static NudFn parse_unr = [](Parser &parser, const Token &token) {
  if constexpr (DEBUG)
    printf("==> unary\n");
  parse_expression(parser, get_rule(token.type).precedence);
  switch (token.type) {
    case Types::SUB:
      parser.chunk.write(Opcodes::OP_NEGATE);
      break;
    case Types::L_NOT:
      parser.chunk.write(Opcodes::OP_NOT);
      break;
    case Types::B_NOT:
      parser.chunk.write(Opcodes::OP_B_NOT);
      break;

    default:
      Logger::fatal("Unknown unary operator");
  }
  if constexpr (DEBUG)
    printf("<== unary\n");
};

static NudFn parse_grp = [](Parser &parser, const Token &) {
  if constexpr (DEBUG)
    printf("==> group\n");
  parse_expression(parser, Precedence::NUL);
  parser.consume(Types::R_PAREN, "Expected ')'");
  if constexpr (DEBUG)
    printf("<== group\n");
};

static LedFn parse_bin = [](Parser &parser, const Token &token) {
  if constexpr (DEBUG)
    printf("==> binary\n");
  parse_expression(parser, get_rule(token.type).precedence);
  switch (token.type) {
    case Types::ADD:
      parser.chunk.write(Opcodes::OP_ADD);
      break;
    case Types::SUB:
      parser.chunk.write(Opcodes::OP_SUB);
      break;
    case Types::MUL:
      parser.chunk.write(Opcodes::OP_MUL);
      break;
    case Types::DIV:
      parser.chunk.write(Opcodes::OP_DIV);
      break;
    default:
      Logger::fatal("Unknown binary operator");
  }
  if constexpr (DEBUG)
    printf("<== binary\n");
};

static LedFn parse_dcl = [](Parser &parser, const Token &) {
  if constexpr (DEBUG)
    printf("==> declaration\n");
  // Logger::print_token(token);

  switch (parser.current_token.type) {
    case Types::NAME: {
      // variable declaration
      parser.consume(Types::NAME, "Expected variable name");
      parser.consume(Types::EQ, "Expected assignment");
      parse_expression(parser, Precedence::NUL);
      parser.symbols.insert("test", 0, 0);
      break;
    }
    case Types::COLON: {
      parser.consume(Types::COLON, "Expected colon");
      parse_expression(parser, Precedence::NUL);
      break;
    }
    case Types::L_PAREN: {
      parser.consume(Types::L_PAREN, "Expected '('");
      parse_expression(parser, Precedence::GRP);
      parser.consume(Types::R_PAREN, "Expected ')'");
      break;
    }
    default: {
      printf("%i\n", static_cast<int>(parser.current_token.type));
      Logger::fatal("Unknown declaration");
    }
  }
  if constexpr (DEBUG)
    printf("<== declaration\n");
};

#define parse_nul nullptr // for code alignment purposes
#define as_int    static_cast<int>

static ParseRule rules[as_int(Types::MAX_TOKENS)] = {
  [as_int(Types::ERROR)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::CMP_EQ)]     = {Precedence::EQT, parse_nul, parse_nul},
  [as_int(Types::CMP_NEQ)]    = {Precedence::EQT, parse_nul, parse_nul},
  [as_int(Types::CMP_GT)]     = {Precedence::CMP, parse_nul, parse_nul},
  [as_int(Types::CMP_GEQ)]    = {Precedence::CMP, parse_nul, parse_nul},
  [as_int(Types::CMP_LT)]     = {Precedence::CMP, parse_nul, parse_nul},
  [as_int(Types::CMP_LEQ)]    = {Precedence::CMP, parse_nul, parse_nul},
  [as_int(Types::CST_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::SET_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::EQ)]         = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::ADD_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::SUB_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::MUL_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::DIV_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::INCR)]       = {Precedence::UNR, parse_nul, parse_nul},
  [as_int(Types::DECR)]       = {Precedence::UNR, parse_nul, parse_nul},
  [as_int(Types::L_AND_EQ)]   = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::L_OR_EQ)]    = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::B_AND_EQ)]   = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::B_OR_EQ)]    = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::B_XOR_EQ)]   = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::POW_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::REM_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::R_SHIFT_EQ)] = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::L_SHIFT_EQ)] = {Precedence::ASG, parse_nul, parse_nul},
  [as_int(Types::AS)]         = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::ASYNC)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::AWAIT)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::CASE)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::CLASS)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::DEFAULT)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::DO)]         = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::ELSE)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::FALSE)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::FOR)]        = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::FROM)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::FX)]         = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::IF)]         = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::IMPORT)]     = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::NEW)]        = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::RETURN)]     = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::SELECT)]     = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::SWITCH)]     = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::TRUE)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::WHILE)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::NUL)]        = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::STR)]        = {Precedence::LIT, parse_lit, parse_nul},
  [as_int(Types::NAME)]       = {Precedence::LIT, parse_dcl, parse_nul},
  [as_int(Types::DEC)]        = {Precedence::LIT, parse_lit, parse_nul},
  [as_int(Types::DOT)]        = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::RANGE_EXCL)] = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::RANGE_L_IN)] = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::RANGE_R_IN)] = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::RANGE_INCL)] = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::SEMICOLON)]  = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::NEWLINE)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::COMMA)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::AT)]         = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::HASH)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::DOLLAR)]     = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::QUERY)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::L_BRACKET)]  = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::R_BRACKET)]  = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::L_PAREN)]    = {Precedence::GRP, parse_grp, parse_nul},
  [as_int(Types::R_PAREN)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::L_CURLY)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::R_CURLY)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::HEX)]        = {Precedence::LIT, parse_lit, parse_nul},
  [as_int(Types::OCTAL)]      = {Precedence::LIT, parse_lit, parse_nul},
  [as_int(Types::BINARY)]     = {Precedence::LIT, parse_lit, parse_nul},
  [as_int(Types::COLON)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::END)]        = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::ADD)]        = {Precedence::TRM, parse_nul, parse_bin},
  [as_int(Types::SUB)]        = {Precedence::TRM, parse_unr, parse_bin},
  [as_int(Types::MUL)]        = {Precedence::FCT, parse_nul, parse_bin},
  [as_int(Types::DIV)]        = {Precedence::FCT, parse_nul, parse_bin},
  [as_int(Types::L_AND)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::L_OR)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::B_AND)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::B_OR)]       = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::CARET)]      = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::B_NOT)]      = {Precedence::UNR, parse_unr, parse_nul},
  [as_int(Types::L_NOT)]      = {Precedence::UNR, parse_unr, parse_nul},
  [as_int(Types::L_SHIFT)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::R_SHIFT)]    = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::POW)]        = {Precedence::NUL, parse_nul, parse_nul},
  [as_int(Types::REM)]        = {Precedence::NUL, parse_nul, parse_nul},
};

ParseRule &get_rule(const Type token_type) {
  return rules[static_cast<int>(token_type)];
}

void parse_expression(Parser &parser, const Precedence precedence) {
  const Token token = parser.current_token;
  parser.advance();

  ParseRule rule = get_rule(token.type);
  if (!rule.nud) {
    // Logger::print_token(token);
    Logger::fatal("Unexpected token");
  }
  rule.nud(parser, token);

  if constexpr (DEBUG)
    printf("  precedence: %i %i\n", static_cast<int>(precedence),
      static_cast<int>(get_rule(parser.current_token.type).precedence));

  while (precedence < get_rule(parser.current_token.type).precedence) {
    Token operator_token = parser.current_token;
    parser.advance();
    if (operator_token.type == Types::END) {
      break;
    }

    rule = get_rule(operator_token.type);
    if (!rule.led)
      Logger::fatal("Unexpected operator");

    rule.led(parser, operator_token);
  }
}
