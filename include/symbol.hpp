#ifndef HADRON_SYMBOL_H
#define HADRON_SYMBOL_H 1

#include <cstddef>

#define SYMBOL_NAME_LEN   0x20
#define SYMBOL_TABLE_SIZE 0x400

struct Symbol {
  char name[SYMBOL_NAME_LEN]{};
  int  location{0};
  int  type{0};
  bool in_use{false};
};

class SymbolTable {
  Symbol table[SYMBOL_TABLE_SIZE]{};

  static size_t hash(const char *name);

  public:
  bool    insert(const char *name, int location, int type);
  Symbol *lookup(const char *name);
};

#endif
