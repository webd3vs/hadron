#ifndef __LANG_TYPES_H
#define __LANG_TYPES_H 1

#include "util/array.h"
#include <stdarg.h>
#include <string.h>

#define PACKED __attribute__((__packed__))

typedef unsigned char boolean;

#define true (boolean)1
#define false (boolean)0

typedef struct Value {
  boolean error;
  void   *value;
} Value;

typedef enum __attribute__((__packed__)) Types {
  UNDEF,

  // COMPARE
  CMP_EQ,  //* ==
  CMP_GT,  //* >
  CMP_LT,  //* <
  CMP_NEQ, //* !=
  CMP_LEQ, //* <=
  CMP_GEQ, //* >=

  // ASSIGN
  CST_EQ,    //* #=
  SET_EQ,    //* $=
  EQ,        //* =
  ADD_EQ,    //* +=
  SUB_EQ,    //* -=
  MUL_EQ,    //* *=
  DIV_EQ,    //* /=
  INCR,      //* ++
  DECR,      //* --
  LAND_EQ,   //* &&=
  LOR_EQ,    //* ||=
  BAND_EQ,   //* &=
  BOR_EQ,    //* |=
  BXOR_EQ,   //* ^=
  POW_EQ,    //* **=
  REM_EQ,    //* %=
  RSHIFT_EQ, //* >>=
  LSHIFT_EQ, //* <<=

  // KEYWORD
  AS,
  ASYNC,
  AWAIT,
  CASE,
  CLASS,
  DEFAULT,
  DO,
  ELSE,
  FALSE,
  FOR,
  FROM,
  FUNC,
  FX,  //* for future use
  FXN, //* ^-------------
  IF,
  IMPORT,
  NEW,
  RETURN,
  SWITCH,
  TRUE,
  WHILE,
  _NULL,

  // TYPE
  STR,      //* "hello world"
  NAME,     //* var_name
  DEC,      //* 123
  DOT,      //* .
  DELMT,    //* ;
  NEWLINE,  //* \n todo remove
  SEP,      //* ,
  AT,       //* @
  HASH,     //* #
  DOLLAR,   //* $
  QUERY,    //* ?
  LBRACKET, //* [
  RBRACKET, //* ]
  LPAREN,   //* (
  RPAREN,   //* )
  LCURLY,   //* {
  RCURLY,   //* }
  HEX,      //* 0xff
  OCTAL,    //* 0o77
  BIN,      //* 0b1111
  COLON,    //* :
  _EOF,     //* \0

  // OPERATION
  ADD,    //* +
  SUB,    //* -
  MUL,    //* *
  DIV,    //* /
  LAND,   //* &&
  LOR,    //* ||
  BAND,   //* &
  BOR,    //* |
  BXOR,   //* ^
  BNOT,   //* ~ (unary)
  LNOT,   //* ! (unary)
  LSHIFT, //* <<
  RSHIFT, //* >>
  POW,    //* **
  REM     //* %
} Type;

typedef struct Position {
  int line;
  int start;
  int end;
  int absStart;
  int absEnd;
} Position;

typedef struct Token {
  Type     type;
  Position pos;
} Token;

typedef enum PACKED AST_Types {
  PROGRAM,
  IMPORT_DECLARATION,
  IMPORT_SPECIFIER,
  ASSIGNMENT_EXPRESSION,
  CALL_EXPRESSION,
  BINARY_EXPRESSION,
  UNARY_EXPRESSION,
  FUNCTION_DECLARATION,
  ASYNC_FUNCTION_DECLARATION,
  CLASS_DECLARATION,
  LITERAL,
  STRING_LITERAL,
  IDENTIFIER,
  TYPED_IDENTIFIER,
  EXPRESSION_STATEMENT,
  RETURN_STATEMENT,
  SWITCH_STATEMENT,
  CASE_STATEMENT,
  DEFAULT_STATEMENT,
  FOR_STATEMENT,
  WHILE_STATEMENT,
} AST_Type;

typedef struct Typed {
  AST_Type type;
} Typed;

typedef struct Identifier {
  AST_Type type;
  char    *name;
} Identifier;

typedef struct TypedIdentifier {
  AST_Type type;
  char    *name;
  char    *kind;
} TypedIdentifier;

typedef struct Literal {
  AST_Type type;
  char    *value;
} Literal;

typedef struct StringLiteral {
  AST_Type type;
  char    *value;
} StringLiteral;

typedef struct Program {
  AST_Type type;
  Array   *body;
} Program;

typedef struct ImportSpecifier {
  AST_Type    type;
  Identifier *name;
  Identifier *local;
} ImportSpecifier;

typedef struct ImportDeclaration {
  AST_Type       type;
  StringLiteral *source;
  Array         *specifiers;
} ImportDeclaration;

typedef struct FunctionDeclaration {
  AST_Type    type;
  boolean     async;
  Identifier *name;
  Array      *params;
  Array      *body;
} FunctionDeclaration;

typedef struct ClassDeclaration {
  AST_Type    type;
  Identifier *name;
  Array      *body;
} ClassDeclaration;

typedef struct CallExpression {
  AST_Type    type;
  Identifier *callee;
  Array      *params;
} CallExpression;

typedef struct ExpressionStatement {
  AST_Type type;
  Typed   *expr;
} ExpressionStatement;

typedef struct ReturnStatement {
  AST_Type type;
  Typed   *expr;
} ReturnStatement;

typedef struct SwitchStatement {
  AST_Type type;
  Typed   *expr;
} SwitchStatement;

typedef struct CaseStatement {
  AST_Type type;
  Typed   *test;
  Array   *body;
} CaseStatement;

typedef struct DefaultStatemtent {
  AST_Type type;
  Array   *body;
} DefaultStatement;

typedef struct ForStatement {
  AST_Type type;
  Typed   *init;
  Typed   *test;
  Typed   *update;
  Array   *body;
} ForStatement;

typedef struct WhileStatement {
  AST_Type type;
  Typed   *test;
  Array   *body;
} WhileStatement;

typedef enum PACKED AssignmentOperators {
  ASGN_UNDEF,
  ASGN_EQ,
  ASGN_ADD,
  ASGN_SUB,
  ASGN_MUL,
  ASGN_DIV,
  ASGN_LAND,
  ASGN_LOR,
  ASGN_BAND,
  ASGN_BOR,
  ASGN_BXOR,
  ASGN_REM,
  ASGN_RSHIFT,
  ASGN_LSHIFT
} AssignmentOperator;

typedef struct AssignmentExpression {
  AST_Type           type;
  AssignmentOperator oper;
  Typed             *left;
  Typed             *right;
} AssignmentExpression;

typedef enum PACKED BinaryOperators {
  BIN_UNDEF,
  BIN_ADD,
  BIN_SUB,
  BIN_MUL,
  BIN_DIV,
  BIN_LAND,
  BIN_LOR,
  BIN_BAND,
  BIN_BOR,
  BIN_BXOR,
  BIN_REM,
  BIN_RSHIFT,
  BIN_LSHIFT,
  BIN_POW,
  // temp
  BIN_EQ,
  BIN_GT,
  BIN_LT,
  BIN_NEQ,
  BIN_LEQ,
  BIN_GEQ
} BinaryOperator;

typedef struct BinaryExpression {
  AST_Type       type;
  BinaryOperator oper;
  Typed         *left;
  Typed         *right;
} BinaryExpression;

typedef enum PACKED UnaryOperators {
  UNARY_UNDEF,
  UNARY_BNOT,
  UNARY_LNOT,
  UNARY_NEG,
  UNARY_INCR,
  UNARY_DECR
} UnaryOperator;

typedef struct UnaryExpression {
  AST_Type      type;
  UnaryOperator oper;
  Typed        *expr;
} UnaryExpression;

typedef struct Result {
  Array *errors;
  void  *data;
} Result;

extern char *getTokenContent(char *code, Token *);
extern void  freeProgram(Program *);

extern Program             *initProgram(int);
extern FunctionDeclaration *initFunctionDeclaration(
  boolean async, Identifier *name, Array *params, Array *body);
extern ClassDeclaration *initClassDeclaration(Identifier *name, Array *body);
extern ImportSpecifier  *initImportSpecifier(
   Identifier *name, Identifier *local_name);
extern ImportDeclaration *initImportDeclaration(
  StringLiteral *, Array *import_specifier_array);
extern AssignmentExpression *initAssignmentExpression(
  Typed *left, Typed *right, AssignmentOperator oper);
extern Literal          *initLiteral(char *value);
extern StringLiteral    *initStringLiteral(char *value);
extern Identifier       *initIdentifier(char *name);
extern TypedIdentifier  *initTypedIdentifier(char *name, char *kind);
extern CallExpression   *initCallExpression(Identifier *callee, Array *params);
extern BinaryExpression *initBinaryExpression(
  BinaryOperator oper, Typed *left, Typed *right);
extern UnaryExpression *initUnaryExpression(UnaryOperator oper, Typed *expr);
extern ExpressionStatement *initExpressionStatement(Typed *expr);
extern ReturnStatement     *initReturnStatement(Typed *expr);
extern SwitchStatement     *initSwitchStatement(Typed *expr);
extern CaseStatement       *initCaseStatement(Typed *test, Array *body);
extern DefaultStatement    *initDefaultStatement(Array *body);
extern ForStatement        *initForStatement(
         Typed *init, Typed *test, Typed *update, Array *body);
extern WhileStatement *initWhileStatement(Typed *test, Array *body);

#endif
