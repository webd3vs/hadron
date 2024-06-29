#ifndef __LANG_PARSER
#define __LANG_PARSER 1

#include "errors.h"
#include "types.h"
#include "util/print.h"
#include "util/str.h"

extern Result *parse(char *code, Array *tokens, char *filename);

#endif
