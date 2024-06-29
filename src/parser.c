#include "parser.h"

#include "types.h"
#include "util/array.h"
#include "util/print.h"
#include "util/stack.h"
#include "util/str.h"
#include <stdio.h>
#include <stdlib.h>

static void initParser(char *, Array *, char *);
static void synchronize();
// go to nearest delimiter or newline
static boolean psync();
static boolean end(void);
static boolean start(void);
static Token  *next(void);
static Token  *prev(void);
static Token  *peekNext(void);
static Token  *current(void);
static boolean match(Type);
static boolean nextIs(Type);
static boolean check(Value *);
static boolean matchAny(int, ...);
static Value  *fmatch(Type, char *);
static Value  *omatch(Type);

static void  *freeValue(void *);
static Value *toValue(void *);

static void delmt(boolean throw);

static Value *parseStatement(void);
static Value *parseForStatement(void);
static Value *parseWhileStatement(void);
static Value *parseIfStatement(void);
static Value *parseExpressionStatement(void);
static Value *parseReturnStatement(void);

static Value *parseDeclaration(void);
static Value *parseImportDeclaration(void);
static Value *parseImportSpecifier(void);
static Value *impSpecArray(void);
static Value *parseFunctionDeclaration(void);
static Value *parseClassDeclaration(void);
static Value *parseAssignmentExpression(Value *);
static Value *parseCallExpression(Value *);
static Value *parseExpression(void);
static Value *parseIdentifier(Value *);
static Value *parseLiteral(void);
static Value *parseStringLiteral(void);

static struct Parser {
  int      iterator;
  Array   *tokens;
  Program *program;
  Array   *errors;
  Array   *callstack;
  char    *code;
  char    *file;
} parser;

void initParser(char *code, Array *tokens, char *file) {
  parser.tokens    = tokens;
  parser.file      = file;
  parser.program   = initProgram(tokens->length / 4);
  parser.callstack = newArray(10);
  parser.errors    = newArray(2);
  parser.iterator  = -1;
  parser.code      = code;
}

// Value *handleError(Value *value) {
//   if (check(value)) {
//     freeValue(value);
//     return NULL;
//   }
//   return value;
// }

void *freeValue(void *value) {
  if (value) free(value);
  return NULL;
}

Value *fmatch(Type type, char *error_string) {
  Value *r = malloc(sizeof(Value));
  if (match(type)) {
    r->error = false;
    r->value = current();
  } else {
    r->error = true;
    r->value = error("Parse", parser.file, error_string, next());
  }
  return r;
}

Value *omatch(Type type) {
  if (match(type)) return toValue(current());
  return NULL;
}

boolean end() { return parser.iterator > parser.tokens->length - 3; }

boolean start() { return parser.iterator < 1; }

Token *next() {
  if (end()) return NULL;
  return parser.tokens->array[++parser.iterator];
}

Token *prev() {
  if (start()) return NULL;
  return parser.tokens->array[--parser.iterator];
}

Token *peekNext() {
  if (end()) {
    Token *t = malloc(sizeof(Token));
    t->type  = _EOF;
    t->pos   = (Position){0, 0, 0, 0, 0};
    return t;
  }
  return parser.tokens->array[parser.iterator + 1];
}

Token *current() { return parser.tokens->array[parser.iterator]; }

// ! avoid using this function as it may cause overhead
boolean matchAny(int n, ...) {
  va_list v;
  va_start(v, n);
  for (int i = 0; i < n; i++) {
    if (match(va_arg(v, int))) {
      va_end(v);
      return true;
    }
  };
  va_end(v);
  return false;
}

boolean match(Type type) {
  if (nextIs(type)) return next(), true;
  return false;
}

inline boolean nextIs(Type type) {
  return peekNext() && peekNext()->type == type;
}

boolean check(Value *r) {
  if (!r || !r->value) return true;
  else if (r->error) {
    pushArray(parser.errors, r->value);
    return true;
  }
  return false;
}

void delmt(boolean throw) {
  if (match(DELMT)) return;
  if (current() && peekNext() && current()->pos.line != peekNext()->pos.line)
    return;
  if (throw)
    pushArray(parser.errors,
      error("Parse", parser.file, "expected delimiter", current()));
}

boolean psync() {
  debug_log("\x1b[91m      psync\x1b[m", parser.code, peekNext());

  if (end()) return false;

  next();
  while (!end() && peekNext()->type != DELMT &&
         current()->pos.line == peekNext()->pos.line)
    next();

  return true;
}

Value *parseClassDeclaration() {
  Value *n = parseIdentifier(NULL);
  if (check(n)) return freeValue(n);
  if (match(COLON)) {
    Value *p = parseIdentifier(NULL);
    if (check(p)) return freeValue(n), freeValue(p);
  }
  return NULL;
  // return initClassDeclaration(initIdentifier(
  // 	getTokenContent(parser.code, n->value)
  // ), NULL);
}

Value *funcParam() {
  Value *a = fmatch(NAME, "expected identifier");
  if (check(a)) return freeValue(a);
  Value *b = omatch(NAME);
  return toValue(
    b ? (void *)initTypedIdentifier(getTokenContent(parser.code, b->value),
          getTokenContent(parser.code, a->value))
      : (void *)initIdentifier(getTokenContent(parser.code, a->value)));
}

Value *funcParams() {
  Array *a = newArray(2);
  if (match(RPAREN)) return toValue(a);

  Value *f = funcParam();
  if (check(f)) return freeArray(a), freeValue(f);
  pushArray(a, f->value);
  while (match(SEP)) {
    Value *s = funcParam();
    if (check(s)) return freeArray(a), freeValue(s);
    pushArray(a, s->value);
  };
  Value *e = fmatch(RPAREN, "expected )");
  if (check(e)) return freeArray(a), freeValue(e);
  trimArray(a);
  return toValue(a);
}

Value *parseBlock() {
  Array *a = newArray(2);
  while (!match(RCURLY)) {
    Value *v = parseDeclaration();
    printf("v: %p\n", (void *)v);
    if (check(v)) {
      if (psync()) freeArray(a);
      else
        return freeValue(v), toValue(a);
    } else
      pushArray(a, v->value);
  };
  trimArray(a);
  return toValue(a);
}

Value *parseFunctionDeclaration() {
  debug_log("func decl", parser.code, peekNext());
  boolean a = false;
  if (match(ASYNC)) a = true;
  Value *f = fmatch(FUNC, "expected `func`");
  if (check(f)) return freeValue(f);
  free(f);
  // Value *n = identifier(NULL, false);
  // if (!n) return pushArray(parser.errors, error("Parse", parser.file, "wtf,
  // no identifier", current())), NULL;
  Value *n = fmatch(NAME, "expected identifier");
  if (check(n)) return freeValue(n);
  Value *lp     = omatch(LPAREN);
  Value *params = NULL;
  if (lp) params = funcParams();
  free(lp);
  Value *b = fmatch(LCURLY, "expected {");
  if (check(b)) return freeValue(n), freeValue(params);

  if (match(RCURLY))
    return toValue(initFunctionDeclaration(a,
      initIdentifier(getTokenContent(parser.code, n->value)),
      params ? params->value : NULL, NULL));
  free(b);
  Value *body = parseBlock();
  if (check(body)) return freeValue(body), freeValue(n), freeValue(params);

  return toValue(initFunctionDeclaration(a,
    initIdentifier(getTokenContent(parser.code, n->value)),
    params ? params->value : NULL, body ? body->value : NULL));
}

Value *parseAssignmentExpression(Value *t) {
  prev();
  Value *n = parseIdentifier(t);
  if (check(n)) return freeValue(t), freeValue(n);
  // Value *q = fmatch(EQ, "expected =");
  next(); // consume operator
  // if (check(q)) return freeValue(t), freeValue(n), freeValue(q);
  Value *r = parseExpression();
  if (check(r)) return freeValue(t), freeValue(n), freeValue(r);
  return toValue(initAssignmentExpression(n->value, r->value, 0));
}

Value *parseIdentifier(Value *t) {
  debug_log("iden", parser.code, peekNext());
  Value *id = fmatch(NAME, "expected identifier");
  if (check(id)) return freeValue(id), freeValue(t);
  if (t && t->value)
    return toValue(initTypedIdentifier(getTokenContent(parser.code, id->value),
      getTokenContent(parser.code, t->value)));
  else {
    return toValue(initIdentifier(getTokenContent(parser.code, id->value)));
  }
}
Value *parseStringLiteral() {
  return toValue(initStringLiteral(getTokenContent(parser.code, current())));
}
Value *parseLiteral() {
  debug_log("lit", parser.code, peekNext());
  if (match(STR)) return parseStringLiteral();
  if (match(LBRACKET)) {
    Value *e = parseExpression();
    while (match(SEP)) {
      parseExpression();
    }
    fmatch(RBRACKET, "expected ]");
    return e;
  }
  if (matchAny(14, FOR, CLASS, FUNC, WHILE, IF, DO, ELSE, FROM, IMPORT, NEW,
        AWAIT, AS, ASYNC, RETURN)) {
    debug_log("---lit", parser.code, peekNext());
    Value *v = malloc(sizeof(Value));
    v->error = true;
    v->value = error("Parse", parser.file, "unexpected keyword", current());
    return v;
  }
  if (!matchAny(6, TRUE, FALSE, DEC, HEX, OCTAL, BIN)) return NULL;
  return toValue(initLiteral(getTokenContent(parser.code, peekNext())));
}
Value *varDecl(Value *t) {
  Value *n = parseIdentifier(t);
  if (check(n)) return freeValue(t), freeValue(n);
  Value *q = fmatch(EQ, "expected =");
  if (check(q)) return freeValue(t), freeValue(n), freeValue(q);
  Value *r = parseExpression();
  if (check(r)) return freeValue(t), freeValue(n), freeValue(q), freeValue(r);
  match(DELMT);
  return toValue(initAssignmentExpression(n->value, r->value, ASGN_EQ));
}

//* working as expected
Value *parseCallExpression(Value *name) {
  debug_log("callExpr", parser.code, peekNext());
  Array *a = newArray(2);
  // if (match(RPAREN)) return toValue(initCallExpression(
  // 	initIdentifier(getTokenContent(parser.code, name->value)), NULL
  // ));
  match(LPAREN);
  while (!match(RPAREN)) {
    Value *e = parseExpression();
    if (check(e)) return freeArray(a), freeValue(e);
    pushArray(a, e->value);
    if (match(RPAREN)) break;
    if (match(SEP)) continue;
    Value *r = fmatch(RPAREN, "expected )");
    if (check(r)) return freeArray(a), freeValue(r);
  }

  return toValue(initCallExpression(
    initIdentifier(getTokenContent(parser.code, name->value)), a));
}

Value *binaryExpr(Value *a) {
  BinaryOperator oper = BIN_UNDEF;
  switch (current()->type) {
    case ADD:
      oper = BIN_ADD;
      break;
    case SUB:
      oper = BIN_SUB;
      break;
    case MUL:
      oper = BIN_MUL;
      break;
    case DIV:
      oper = BIN_DIV;
      break;
    case LOR:
      oper = BIN_LOR;
      break;
    case BOR:
      oper = BIN_BOR;
      break;
    case REM:
      oper = BIN_REM;
      break;
    case POW:
      oper = BIN_POW;
      break;
    case LAND:
      oper = BIN_LAND;
      break;
    case BAND:
      oper = BIN_BAND;
      break;
    case BXOR:
      oper = BIN_BXOR;
      break;
    case RSHIFT:
      oper = BIN_RSHIFT;
      break;
    case LSHIFT:
      oper = BIN_RSHIFT;
      break;
    case CMP_EQ:
      oper = BIN_EQ;
      break;
    case CMP_GT:
      oper = BIN_GT;
      break;
    case CMP_LT:
      oper = BIN_LT;
      break;
    case CMP_NEQ:
      oper = BIN_NEQ;
      break;
    case CMP_LEQ:
      oper = BIN_LEQ;
      break;
    case CMP_GEQ:
      oper = BIN_GEQ;
      break;
    default:
      oper = BIN_UNDEF;
      break;
  }
  if (oper == BIN_UNDEF)
    pushArray(
      parser.errors, error("Parse", parser.file, "internal error", current()));
  Value *b = parseExpression();
  if (check(b)) return freeValue(a);
  return toValue(initBinaryExpression(oper,
    (Typed *)initLiteral(getTokenContent(parser.code, a->value)), b->value));
}

Value *parseExpression() {
  debug_log("expr", parser.code, peekNext());
  //! operator precedence:
  // 15 ,                          LTR
  // 14 x=                         RTL
  // 13 ?: comparison-chaining
  // 12 ||                         LTR
  // 11 &&
  // 10 |
  // 9  ^
  // 8  &
  // 7  == !=
  // 6  < <= > >=
  // 5  << >>
  // 4  + -
  // 3  * / %
  // 2  prefix ++ -- ! ~            RTL
  // 1  suffix ++ -- () []          LTR

  if (match(NAME)) {
    // todo convert to a switch statement, too many cases to handle for ifs

    Value *t = toValue(current());
    if (nextIs(NAME)) {
      return varDecl(t);
    } else if (nextIs(LPAREN)) {
      return parseCallExpression(t);
    }
    // else if (nextIs(DOT)) return chainExpr();
    else if (matchAny(2, SET_EQ, CST_EQ)) {
      prev();
      return parseAssignmentExpression(t);
    } else if (matchAny(3, EQ, ADD_EQ, SUB_EQ)) {
      prev();
      return parseAssignmentExpression(t);
    } else if (matchAny(6, CMP_EQ, CMP_GT, CMP_LT, CMP_NEQ, CMP_LEQ, CMP_GEQ)) {
      return binaryExpr(t);
    } else if (matchAny(2, INCR, DECR)) {
      return toValue(
        initUnaryExpression(current()->type == INCR ? UNARY_INCR : UNARY_DECR,
          (Typed *)initLiteral(getTokenContent(parser.code, t->value))));
    } else {
      return prev(), parseIdentifier(NULL);
    }
  } else if (matchAny(4, DEC, HEX, OCTAL, BIN)) {
    Value *l = toValue(current());
    if (matchAny(13, ADD, SUB, MUL, DIV, LAND, LOR, BAND, BOR, BXOR, REM,
          RSHIFT, LSHIFT, POW))
      return binaryExpr(l);

    return freeValue(l),
           toValue(initLiteral(getTokenContent(parser.code, current())));
  } else if (match(STR)) {
    Value *l = toValue(current());

    // todo add binary expression

    return freeValue(l), parseStringLiteral();
  }
  return parseLiteral();
}

Value *toValue(void *x) {
  if (!x) return NULL;
  Value *r = malloc(sizeof(Value));
  r->error = false;
  r->value = x;
  return r;
}

Value *parseImportSpecifier() {
  Value *n = fmatch(NAME, "unexpected token (9)");
  if (check(n)) return freeValue(n);
  if (!match(AS))
    return toValue(initImportSpecifier(
      initIdentifier(getTokenContent(parser.code, n->value)),
      initIdentifier(getTokenContent(parser.code, n->value))));
  Value *ln = fmatch(NAME, "expected identifier");
  if (check(ln)) return freeValue(ln);
  return toValue(
    initImportSpecifier(initIdentifier(getTokenContent(parser.code, n->value)),
      initIdentifier(getTokenContent(parser.code, ln->value))));
}

Value *impSpecArray() {
  Array *a = newArray(2);
  if (match(RCURLY)) return toValue(a);
  Value *f = parseImportSpecifier();
  if (check(f)) return freeArray(a), freeValue(f);
  pushArray(a, f->value);
  while (match(SEP)) {
    Value *s = parseImportSpecifier();
    if (check(s)) return freeArray(a), freeValue(s);
    pushArray(a, s->value);
  };
  Value *e = fmatch(RCURLY, "expected }");
  if (check(e)) return freeArray(a), freeValue(e);
  trimArray(a);
  return toValue(a);
}

Value *parseImportDeclaration() {
  Value *n = fmatch(STR, "unexpected token (1)");
  if (check(n)) return freeValue(n);
  Value *i = fmatch(IMPORT, "unexpected token (2)");
  if (check(i)) return freeValue(i), freeValue(n);
  if (match(LCURLY)) {
    Value *speca = impSpecArray();
    if (check(speca)) return freeValue(speca), freeValue(n), freeValue(i);
    match(DELMT);
    return toValue(initImportDeclaration(
      initStringLiteral(getTokenContent(parser.code, n->value)), speca->value));
  } else {
    Value *spec = parseImportSpecifier();
    if (check(spec)) return freeValue(spec), freeValue(n), freeValue(i);
    Array *speca = newArray(1);
    pushArray(speca, spec->value);
    match(DELMT);
    return toValue(initImportDeclaration(
      initStringLiteral(getTokenContent(parser.code, n->value)), speca));
  }
}

Value *parseExpressionStatement() {
  debug_log("exprStmt", parser.code, peekNext());
  Value *e = parseExpression();
  if (check(e)) return freeValue(e);
  delmt(false);
  return toValue(initExpressionStatement(e->value));
}

Value *parseIfStatement() {
  // Value *condition = expr();
  return NULL;
} // todo

Value *parseWhileStatement() {
  debug_log("while stmt", parser.code, peekNext());
  match(WHILE); // consume while
  Value *cond = parseExpression();
  Value *b    = fmatch(LCURLY, "expected {");
  if (check(b)) return freeValue(cond), freeValue(b);
  Value *body = parseBlock();
  return toValue(initWhileStatement(cond->value, body->value));
} // todo

Value *parseForStatement() {
  debug_log("for stmt", parser.code, peekNext());
  match(FOR); // consume for
  Value *init = parseExpression();
  delmt(true);
  Value *test = parseExpression();
  delmt(true);
  Value *update = parseExpression();
  Value *b      = fmatch(LCURLY, "expected {");
  if (check(b))
    return freeValue(init), freeValue(test), freeValue(update), freeValue(b);
  Value *body = parseBlock();
  return toValue(
    initForStatement(init->value, test->value, update->value, body->value));
} // todo

Value *parseReturnStatement() {
  debug_log("retstmt", parser.code, peekNext());
  match(RETURN); // consume return
  Value *retval = parseExpression();
  if (check(retval)) return NULL;
  delmt(false);
  return toValue(initReturnStatement(retval->value));
} // todo

Value *parseSwitchStatement() {
  debug_log("switch stmt", parser.code, peekNext());
  match(SWITCH); // consume switch
  Value *v = parseExpression();
  if (check(v)) return freeValue(v);
  delmt(false);
  return toValue(initSwitchStatement(v->value));
}

Value *parseCaseStatement() {
  debug_log("case stmt", parser.code, peekNext());
  match(CASE); // consume case
  Value *t = parseExpression();
  if (check(t)) return freeValue(t);
  Value *b = fmatch(LCURLY, "expected {");
  if (check(b)) return freeValue(t), freeValue(b);
  Value *body = parseBlock();
  return toValue(initCaseStatement(t->value, body->value));
}

Value *parseDefaultStatement() {
  debug_log("default stmt", parser.code, peekNext());
  match(DEFAULT); // consume default
  Value *b = fmatch(LCURLY, "expected {");
  if (check(b)) return freeValue(b);
  Value *body = parseBlock();
  return toValue(initDefaultStatement(body->value));
}

Value *parseStatement() {
  debug_log("stmt", parser.code, peekNext());
  switch (peekNext()->type) {
    case SWITCH:
      return parseSwitchStatement();
    case CASE:
      return parseCaseStatement();
    case DEFAULT:
      return parseDefaultStatement();
    case FOR:
      return parseForStatement();
    case WHILE:
      return parseWhileStatement();
    case RETURN:
      return parseReturnStatement();
    case IF:
      return parseIfStatement();
    default:
      return parseExpressionStatement();
  }
}

Value *parseDeclaration() {
  debug_log("decl", parser.code, peekNext());
  switch (peekNext()->type) {
    case FROM:
      return parseImportDeclaration();
    case FUNC:
    case ASYNC:
      return parseFunctionDeclaration();
    case CLASS:
      return parseClassDeclaration();
    default:
      return parseStatement();
  }
}

void synchronize() {
  debug_log("-- synchronize", parser.code, peekNext());
  while (!end()) {
    if (peekNext()) switch (peekNext()->type) {
        case ASYNC:
        case CLASS:
        case DO:
        case FOR:
        case FROM:
        case FUNC:
        case IF:
        case RETURN:
        case SWITCH:
        case WHILE:
          return;
        case DELMT:
          next();
          return;
        default:
          next();
      };
  }
}

Result *parse(char *code, Array *tokens, char *fname) {
  printf("\n");
  initParser(code, tokens, fname);
  Result *result = malloc(sizeof(Result));
  while (!end()) {
    printf("> loop\n");
    if (match(_EOF)) {
      printf("EOF\n");
      break;
    }
    Value *d = parseDeclaration();
    if (check(d)) synchronize();
    else {
      printf("< pushing value, type: %i\n", ((Typed *)d->value)->type);
      pushArray(parser.program->body, d->value);
    }
  }
  printf("\n");
  trimArray(parser.errors);

  result->errors = parser.errors;
  result->data   = parser.program;

  return result;
}
