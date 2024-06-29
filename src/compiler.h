#ifndef __LANG_COMPILER
#define __LANG_COMPILER 1

#include "errors.h"
#include "types.h"

extern Result *compile(char *code, Program *p, char *filename);

#endif
