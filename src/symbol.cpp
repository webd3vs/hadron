#include "symbol.hpp"

#include <cstring>

size_t SymbolTable::hash(const char *name) {
  size_t hash = 5381;
  while (*name) {
    hash = (hash << 5) + hash + *name; // hash * 33 + c
    ++name;
  }
  return hash % SYMBOL_TABLE_SIZE;
}

bool SymbolTable::insert(
  const char *name, const int location, const SymbolType type) {
  const size_t idx = hash(name);
  for (size_t i = 0; i < SYMBOL_TABLE_SIZE; ++i) {
    const size_t current_idx = (idx + i) % SYMBOL_TABLE_SIZE;
    Symbol      &entry       = table[current_idx];
    if (!entry.in_use) {
      strncpy(entry.name, name, SYMBOL_NAME_LEN - 1);
      entry.name[SYMBOL_NAME_LEN - 1] = '\0'; // Ensure null-termination
      entry.location                  = location;
      entry.type                      = type;
      entry.in_use                    = true;
      return true;
    }
    if (strncmp(entry.name, name, SYMBOL_NAME_LEN) == 0) {
      return false;
    }
  }
  return false;
}

Symbol *SymbolTable::lookup(const char *name) {
  const size_t idx = hash(name);
  for (size_t i = 0; i < SYMBOL_TABLE_SIZE; ++i) {
    const size_t current_idx = (idx + i) % SYMBOL_TABLE_SIZE;
    Symbol      &entry       = table[current_idx];
    if (!entry.in_use) {
      return nullptr; // Not found
    }
    if (strncmp(entry.name, name, SYMBOL_NAME_LEN) == 0) {
      return &entry; // Found
    }
  }
  return nullptr; // Not found
}
