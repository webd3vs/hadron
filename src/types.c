#include "types.h"

void initArray(void *arraylike, int size) {
	Array *arr = (Array *)arraylike;
	arr->array = malloc(sizeof(void *) * size);
	arr->length = 0;
	arr->size = size;
}
Program *initProgram(int size) {
	Program *prg = malloc(sizeof(Program));
	prg->type = PROGRAM;
	Array body;
	initArray(&body, size);
	prg->body = body;
	return prg;
}

void push(void *arraylike, void *item) {
	Array *ptr = (Array *)arraylike;
	if (ptr->size == ptr->length) {
		ptr->array = realloc(ptr->array, sizeof(void *) * (ptr->size*=1.5));
	}
	ptr->array[ptr->length++] = item;
}

void freeArray(void *arraylike) {
	Array *ptr = (Array *)arraylike;
	for (int i = 0; i < ptr->length; i++) free(ptr->array[i]);
	free(ptr->array);
}

void freeProgram(Program *program) {
	for (int i = 0; i < program->body.length; i++) {
		switch (((Typed *)program->body.array[i])->type) {
			case PROGRAM: { break; }
			case IMPORT_DECLARATION: { break; }
			case IMPORT_SPECIFIER: { break; }
		}
	}
}

string getType(Type type, ...) {
	va_list v;
	va_start(v, type);
	string s = va_arg(v, string);
	string r = malloc(strlen(s) + 16);
	strcpy(r, s);
	string s0;
	switch (type) {
		case STR: s0 = "String"; break;
		case NAME: s0 = "Name"; break;
		case DEC: case HEX: case OCTAL: case BIN: s0 = "Number"; break;
		case BRACKET: s0 = "Bracket"; break;
		case PAREN: s0 = "Parenthesis"; break;
		case CBRACKET: s0 = "CurlyBracket"; break;
		default: s0 = "Undefined"; break;
	}
	strcat(r, s0);
	return r;
}

ImportSpecifier *initImportSpecifier(string name, string local) {
	ImportSpecifier *spec = malloc(sizeof(ImportSpecifier));
	spec->type = IMPORT_SPECIFIER;
	spec->name = name;
	spec->local = local;
	return spec;
}
ImportDeclaration *initImportDeclaration(string src, ImportSpecifierArray *specs) {
	ImportDeclaration *decl = malloc(sizeof(ImportSpecifier));
	decl->type = IMPORT_DECLARATION;
	decl->source = src;
	decl->specifiers = specs;
	return decl;
}
