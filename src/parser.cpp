#include "parser.h"
#include "bytecode.h"
#include "types.h"

Parser::Parser(Lexer &lexer, Chunk &chunk) : lexer(lexer), chunk(chunk) {
  advance();
}

void Parser::advance() {
  prev_token    = current_token;
  current_token = lexer.advance();
}

Token &Parser::consume(const Type type, const char *error) {
  if (current_token.type == type) {
    advance();
    return prev_token;
  }
  Logger::fatal(error);
  return current_token; // never reached
}

bool Parser::match(const Type type) {
  if (current_token.type == type) {
    advance();
    return true;
  }
  return false;
}

ParseRule &get_rule(Type token_type);

static NudFn parse_fxn = [](Parser &parser, const Token &) {
  const auto name = static_cast<const char *>(
    parser.consume(Types::NAME, "Expected function name").value.ptr);

  // TODO: Accept parameters
  parser.consume(Types::L_PAREN, "Expected '(' after function name");
  if (!parser.match(Types::R_PAREN)) {
    Logger::fatal("Expected ')' after function name");
  }

  parser.chunk.write(OpCodes::FX_ENTRY);

  // Parse function body
  parser.consume(Types::L_CURLY, "Expected '{' to start function body");
  const size_t start_address = parser.chunk.pos;
  while (!parser.match(Types::R_CURLY) /* && !parser.is_at_end() */) {
    parse_expression(parser, Precedence::NUL);
  }
  // const size_t end_address = parser.chunk.pos;

  // Emit bytecode for function definition
  // parser.chunk.write(static_cast<uint8_t>(parameters.size()));
  // for (const auto &param : parameters) {
  // parser.chunk.write(param.c_str());
  // }
  parser.chunk.write(OpCodes::FX_EXIT);

  // Store function metadata in symbol table
  parser.symbols.insert(
    name, static_cast<int>(start_address), SymbolType::FUNCTION);
};

static NudFn parse_lit = [](Parser &parser, const Token &token) {
  switch (token.type) {
    case Types::DEC:
    case Types::HEX:
    case Types::OCTAL:
    case Types::BINARY:
      parser.chunk.write(OpCodes::MOVE);
      parser.chunk.write(token.index);
      parser.chunk.write(token.value.f64);
      break;
    case Types::STR:
      // parser.chunk.write(token.index);
      parser.symbols.insert(
        static_cast<const char *>(token.value.ptr), 0, SymbolType::STR);
      break;
    default:
      Logger::fatal("Unknown literal");
  }
};

static NudFn parse_unr = [](Parser &parser, const Token &token) {
  parse_expression(parser, get_rule(token.type).precedence);
  switch (token.type) {
    case Types::ADD: // unary + does nothing
      break;
    case Types::SUB:
      parser.chunk.write(OpCodes::NEGATE);
      break;
    case Types::L_NOT:
      parser.chunk.write(OpCodes::NOT);
      break;
    case Types::B_NOT:
      parser.chunk.write(OpCodes::B_NOT);
      break;
    default:
      Logger::fatal("Unknown unary operator");
  }
};

static NudFn parse_grp = [](Parser &parser, const Token &) {
  parse_expression(parser, Precedence::NUL);
  parser.consume(Types::R_PAREN, "Expected ')'");
};

static LedFn parse_bin = [](Parser &parser, const Token &token) {
  parse_expression(parser, get_rule(token.type).precedence);
  switch (token.type) {
    case Types::ADD:
      parser.chunk.write(OpCodes::ADD);
      break;
    case Types::SUB:
      parser.chunk.write(OpCodes::SUB);
      break;
    case Types::MUL:
      parser.chunk.write(OpCodes::MUL);
      break;
    case Types::DIV:
      parser.chunk.write(OpCodes::DIV);
      break;
    case Types::L_AND:
      parser.chunk.write(OpCodes::L_AND);
      break;
    case Types::L_OR:
      parser.chunk.write(OpCodes::L_OR);
      break;
    case Types::B_AND:
      parser.chunk.write(OpCodes::B_AND);
      break;
    case Types::B_OR:
      parser.chunk.write(OpCodes::B_OR);
      break;
    case Types::CARET:
      parser.chunk.write(OpCodes::B_XOR);
      break;
    case Types::POW:
      parser.chunk.write(OpCodes::POW);
      break;
    default:
      Logger::fatal("Unknown binary operator");
  }
};

static LedFn parse_rng = [](Parser &parser, const Token &token) {
  parse_expression(parser, get_rule(token.type).precedence);
  switch (token.type) {
    case Types::RANGE_EXCL:
      parser.chunk.write(OpCodes::RANGE_EXCL);
      break;
    case Types::RANGE_L_IN:
      parser.chunk.write(OpCodes::RANGE_L_IN);
      break;
    case Types::RANGE_R_IN:
      parser.chunk.write(OpCodes::RANGE_R_IN);
      break;
    case Types::RANGE_INCL:
      parser.chunk.write(OpCodes::RANGE_INCL);
      break;
    default:
      Logger::fatal("Unknown range operator");
  }
};

static LedFn parse_dcl = [](Parser &parser, const Token &) {
  switch (parser.current_token.type) {
    case Types::NAME: {
      // variable declaration
      parser.consume(Types::NAME, "Expected variable name");
      parser.consume(Types::EQ, "Expected assignment");
      parse_expression(parser, Precedence::NUL);
      parser.symbols.insert("test", 0, SymbolType::I32);
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
};

#define parse_nul nullptr // for code alignment purposes
#define I         static_cast<int>

static ParseRule rules[I(Types::MAX_TOKENS)] = {
  [I(Types::ERROR)]      = {Precedence::MAX, parse_nul, parse_nul},
  [I(Types::CMP_EQ)]     = {Precedence::EQT, parse_nul, parse_nul},
  [I(Types::CMP_NEQ)]    = {Precedence::EQT, parse_nul, parse_nul},
  [I(Types::CMP_GT)]     = {Precedence::CMP, parse_nul, parse_nul},
  [I(Types::CMP_GEQ)]    = {Precedence::CMP, parse_nul, parse_nul},
  [I(Types::CMP_LT)]     = {Precedence::CMP, parse_nul, parse_nul},
  [I(Types::CMP_LEQ)]    = {Precedence::CMP, parse_nul, parse_nul},
  [I(Types::CST_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::SET_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::EQ)]         = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::ADD_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::SUB_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::MUL_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::DIV_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::INCR)]       = {Precedence::UNR, parse_nul, parse_nul},
  [I(Types::DECR)]       = {Precedence::UNR, parse_nul, parse_nul},
  [I(Types::L_AND_EQ)]   = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::L_OR_EQ)]    = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::B_AND_EQ)]   = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::B_OR_EQ)]    = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::B_XOR_EQ)]   = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::POW_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::REM_EQ)]     = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::R_SHIFT_EQ)] = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::L_SHIFT_EQ)] = {Precedence::ASG, parse_nul, parse_nul},
  [I(Types::AS)]         = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::ASYNC)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::AWAIT)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::CASE)]       = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::CLASS)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::DEFAULT)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::DO)]         = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::ELSE)]       = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::FALSE)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::FOR)]        = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::FROM)]       = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::FX)]         = {Precedence::NUL, parse_fxn, parse_nul},
  [I(Types::IF)]         = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::IMPORT)]     = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::NEW)]        = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::RETURN)]     = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::SELECT)]     = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::SWITCH)]     = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::TRUE)]       = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::WHILE)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::NUL)]        = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::STR)]        = {Precedence::LIT, parse_lit, parse_nul},
  [I(Types::NAME)]       = {Precedence::LIT, parse_dcl, parse_nul},
  [I(Types::DEC)]        = {Precedence::LIT, parse_lit, parse_nul},
  [I(Types::HEX)]        = {Precedence::LIT, parse_lit, parse_nul},
  [I(Types::OCTAL)]      = {Precedence::LIT, parse_lit, parse_nul},
  [I(Types::BINARY)]     = {Precedence::LIT, parse_lit, parse_nul},
  [I(Types::DOT)]        = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::SEMICOLON)]  = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::NEWLINE)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::COMMA)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::AT)]         = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::HASH)]       = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::DOLLAR)]     = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::QUERY)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::L_BRACKET)]  = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::R_BRACKET)]  = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::L_PAREN)]    = {Precedence::GRP, parse_grp, parse_nul},
  [I(Types::R_PAREN)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::L_CURLY)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::R_CURLY)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::COLON)]      = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::END)]        = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::RANGE_EXCL)] = {Precedence::RNG, parse_nul, parse_rng},
  [I(Types::RANGE_L_IN)] = {Precedence::RNG, parse_nul, parse_rng},
  [I(Types::RANGE_R_IN)] = {Precedence::RNG, parse_nul, parse_rng},
  [I(Types::RANGE_INCL)] = {Precedence::RNG, parse_nul, parse_rng},
  [I(Types::ADD)]        = {Precedence::TRM, parse_unr, parse_bin},
  [I(Types::SUB)]        = {Precedence::TRM, parse_unr, parse_bin},
  [I(Types::MUL)]        = {Precedence::FCT, parse_nul, parse_bin},
  [I(Types::DIV)]        = {Precedence::FCT, parse_nul, parse_bin},
  [I(Types::L_AND)]      = {Precedence::LND, parse_nul, parse_bin},
  [I(Types::L_OR)]       = {Precedence::LOR, parse_nul, parse_bin},
  [I(Types::B_AND)]      = {Precedence::BND, parse_nul, parse_bin},
  [I(Types::B_OR)]       = {Precedence::BOR, parse_nul, parse_bin},
  [I(Types::CARET)]      = {Precedence::XOR, parse_nul, parse_bin},
  [I(Types::B_NOT)]      = {Precedence::UNR, parse_unr, parse_nul},
  [I(Types::L_NOT)]      = {Precedence::UNR, parse_unr, parse_nul},
  [I(Types::L_SHIFT)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::R_SHIFT)]    = {Precedence::NUL, parse_nul, parse_nul},
  [I(Types::POW)]        = {Precedence::EXP, parse_nul, parse_bin},
  [I(Types::REM)]        = {Precedence::NUL, parse_nul, parse_nul},
};
#undef I
#undef parse_nul

ParseRule &get_rule(const Type token_type) {
  return rules[static_cast<int>(token_type)];
}

void parse_expression(Parser &parser, const Precedence precedence) {
  const Token token = parser.current_token;
  parser.advance();

  ParseRule rule = get_rule(token.type);
  if (!rule.nud) {
    Logger::print_token(token);
    Logger::fatal("Unexpected token");
  }
  rule.nud(parser, token);

  while (precedence < get_rule(parser.current_token.type).precedence) {
    Token operator_token = parser.current_token;
    parser.advance();
    if (operator_token.type == Types::END) {
      break;
    }

    rule = get_rule(operator_token.type);
    if (!rule.led) {
      Logger::print_token(operator_token);
      Logger::fatal("Unexpected operator");
    }

    rule.led(parser, operator_token);
  }
}
