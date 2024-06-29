#include "compiler.h"
#include "types.h"
#include "util/array.h"
#include "util/str.h"
#include <stdio.h>
#include <string.h>

static struct Compiler {
  Program *program;
  Array   *errors;
  FILE    *out;
  char    *filename;
  char    *code; // do I need this ?
} compiler;

void initCompiler(char *code, Program *p, char *filename) {
  compiler.code     = code;
  compiler.program  = p;
  compiler.filename = filename;
}

void tab(int indent) {
  for (int i = 0; i < indent; i++)
    fprintf(compiler.out, "  ");
}

static char *getBinaryOperator(BinaryOperator oper) {
  switch (oper) {
    case BIN_ADD:
      return "+";
    case BIN_SUB:
      return "-";
    case BIN_MUL:
      return "*";
    case BIN_DIV:
      return "/";
    case BIN_LAND:
      return "&&";
    case BIN_LOR:
      return "||";
    case BIN_BAND:
      return "&";
    case BIN_BOR:
      return "|";
    case BIN_BXOR:
      return "^";
    case BIN_REM:
      return "%";
    case BIN_RSHIFT:
      return ">>";
    case BIN_LSHIFT:
      return "<<";
    case BIN_POW:
      return "**";

    case BIN_EQ:
      return "==";
    case BIN_GT:
      return ">";
    case BIN_LT:
      return "<";
    case BIN_NEQ:
      return "!=";
    case BIN_LEQ:
      return "<=";
    case BIN_GEQ:
      return ">=";
    default:
      return "";
  }
}

static char *getUnaryOperator(UnaryOperator oper) {
  switch (oper) {
    case UNARY_INCR:
      return "++";
    case UNARY_DECR:
      return "--";
    case UNARY_LNOT:
      return "!";
    case UNARY_BNOT:
      return "~";
    case UNARY_NEG:
      return "-";
    default:
      return "";
  }
}

static char *getKind(char *kind) {
  if (strcmp(kind, "i32") == 0) {
    return "int";
  }
  if (strcmp(kind, "strp") == 0) {
    return "char **";
  }
  return kind;
}

static void compile_internal(Typed *v, int indent) {
  if (v == NULL) return;

  switch (v->type) {
    case EXPRESSION_STATEMENT: {
      ExpressionStatement *stmt = (ExpressionStatement *)v;
      tab(indent);
      compile_internal(stmt->expr, indent + 1);
      fprintf(compiler.out, ";\n");
      break;
    }
    case RETURN_STATEMENT: {
      ReturnStatement *stmt = (ReturnStatement *)v;
      tab(indent);
      fprintf(compiler.out, "return ");
      compile_internal(stmt->expr, indent + 1);
      fprintf(compiler.out, ";\n");
      break;
    }
    case SWITCH_STATEMENT: {
      SwitchStatement *stmt = (SwitchStatement *)v;
      tab(indent);
      fprintf(compiler.out, "switch (");
      compile_internal((Typed *)stmt->expr, indent + 1);
      fprintf(compiler.out, ") {\n");
      break;
    }
    case CASE_STATEMENT: {
      CaseStatement *stmt = (CaseStatement *)v;
      tab(indent + 1);
      fprintf(compiler.out, "case ");
      compile_internal((Typed *)stmt->test, indent + 1);
      fprintf(compiler.out, ":\n");
      for (int i = 0; i < stmt->body->length; i++) {
        compile_internal(stmt->body->array[i], indent + 2);
      };
      tab(indent + 2);
      fprintf(compiler.out, "break;\n");
      break;
    }
    case DEFAULT_STATEMENT: {
      DefaultStatement *stmt = (DefaultStatement *)v;
      tab(indent + 1);
      fprintf(compiler.out, "default:\n");
      for (int i = 0; i < stmt->body->length; i++) {
        compile_internal(stmt->body->array[i], indent + 2);
      };
      tab(indent + 2);
      fprintf(compiler.out, "break;\n");
      tab(indent);
      fprintf(compiler.out, "}\n");
      break;
    }
    case FOR_STATEMENT: {
      ForStatement *stmt = (ForStatement *)v;
      tab(indent);
      fprintf(compiler.out, "for (");
      compile_internal((Typed *)stmt->init, indent + 1);
      fprintf(compiler.out, "; ");
      compile_internal((Typed *)stmt->test, indent + 1);
      fprintf(compiler.out, "; ");
      compile_internal((Typed *)stmt->update, indent + 1);
      fprintf(compiler.out, ") {\n");
      if (stmt->body)
        for (int i = 0; i < stmt->body->length; i++) {
          compile_internal(stmt->body->array[i], indent + 1);
        };
      tab(indent);
      fprintf(compiler.out, "}\n");
      break;
    }
    case WHILE_STATEMENT: {
      WhileStatement *stmt = (WhileStatement *)v;
      tab(indent);
      fprintf(compiler.out, "while (");
      compile_internal((Typed *)stmt->test, indent + 1);
      fprintf(compiler.out, ") {\n");
      if (stmt->body)
        for (int i = 0; i < stmt->body->length; i++) {
          compile_internal(stmt->body->array[i], indent + 1);
        };
      tab(indent);
      fprintf(compiler.out, "}\n");
      break;
    }
    case CALL_EXPRESSION: {
      CallExpression *expr = (CallExpression *)v;
      compile_internal((Typed *)expr->callee, indent);
      fprintf(compiler.out, "(");
      int len = expr->params->length;
      for (int i = 0; i < len; i++) {
        compile_internal(expr->params->array[i], indent + 1);
        fprintf(compiler.out, i < len - 1 ? ", " : "");
      };
      fprintf(compiler.out, ")");
      break;
    }
    case BINARY_EXPRESSION: {
      BinaryExpression *expr = (BinaryExpression *)v;
      compile_internal((Typed *)expr->left, indent + 1);
      fprintf(compiler.out, " %s ", getBinaryOperator(expr->oper));
      compile_internal((Typed *)expr->right, indent + 1);
      break;
    }
    case UNARY_EXPRESSION: {
      UnaryExpression *expr = (UnaryExpression *)v;
      compile_internal((Typed *)expr->expr, indent + 1);
      fprintf(compiler.out, "%s", getUnaryOperator(expr->oper));
      break;
    }
    case STRING_LITERAL: {
      StringLiteral *ltr = (StringLiteral *)v;
      fprintf(compiler.out, "\"%s\"", ltr->value);
      break;
    }
    case LITERAL: {
      Literal *ltr = (Literal *)v;
      fprintf(compiler.out, "%s", ltr->value);
      break;
    }
    case IDENTIFIER: {
      Identifier *id = (Identifier *)v;
      fprintf(compiler.out, "%s", id->name);
      break;
    }
    case TYPED_IDENTIFIER: {
      TypedIdentifier *id = (TypedIdentifier *)v;

      char *kind = id->kind;
      char *name = id->name;
      if (strcmp(kind, name)) {
        fprintf(compiler.out, "%s ", getKind(id->kind));
        fprintf(compiler.out, "%s", id->name);
      } else {
        fprintf(compiler.out, "%s", id->name);
      }
      break;
    }
    case PROGRAM: {
      Program *p = (Program *)v;
      for (int i = 0; i < p->body->length; i++) {
        compile_internal(p->body->array[i], indent);
      };
      break;
    }
    case IMPORT_DECLARATION: {
      ImportDeclaration *decl = (ImportDeclaration *)v;
      compile_internal((Typed *)decl->source, 0);
      for (int i = 0; i < decl->specifiers->length; i++) {
        compile_internal(decl->specifiers->array[i], indent + 1);
      };
      break;
    }
    case FUNCTION_DECLARATION: {
      FunctionDeclaration *decl = (FunctionDeclaration *)v;
      tab(indent);
      fprintf(compiler.out, "int ");
      compile_internal((Typed *)decl->name, 0);

      fprintf(compiler.out, "(");
      int len = decl->params->length;
      if (decl->params)
        for (int i = 0; i < len; i++) {
          compile_internal(decl->params->array[i], indent + 1);
          fprintf(compiler.out, i < len - 1 ? ", " : "");
        };
      fprintf(compiler.out, ") {\n");
      if (decl->body)
        for (int i = 0; i < decl->body->length; i++) {
          compile_internal(decl->body->array[i], indent + 1);
        };
      tab(indent);
      fprintf(compiler.out, "}\n");
      break;
    }
    case IMPORT_SPECIFIER: {
      ImportSpecifier *spec = (ImportSpecifier *)v;
      compile_internal((Typed *)spec->name, 0);
      compile_internal((Typed *)spec->local, 0);
      break;
    }
    case ASSIGNMENT_EXPRESSION: {
      AssignmentExpression *expr = (AssignmentExpression *)v;
      fprintf(compiler.out, "int ");
      compile_internal((Typed *)expr->left, 0);
      fprintf(compiler.out, " = ");
      compile_internal((Typed *)expr->right, indent + 1);
      break;
    }
    default:
      break;
  }
}

Result *compile(char *code, Program *p, char *filename) {
  initCompiler(code, p, filename);
  Result *result = malloc(sizeof(Result));

  char *outname = utfcat(filename, ".c");

  FILE *out = fopen(outname, "w");
  if (out == NULL) {
    pushArray(compiler.errors, clierror(2, "could not open file for writing"));
    result->errors = compiler.errors;
    result->data   = compiler.program;
    return result;
  }

  compiler.out = out;

  fprintf(out, "#include <stdio.h>\n\n");

  compile_internal((Typed *)compiler.program, 0);

  result->errors = compiler.errors;
  result->data   = compiler.program;

  return result;
}
