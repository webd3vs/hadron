#include "arguments.hpp"
#include "bytecode.hpp"
#include "file.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "vm.hpp"

#include <cstring>
#include <iostream>

#define MAX_EXT_LENGTH      0x10
#define MAX_DIR_LENGTH      0x100
#define MAX_FILENAME_LENGTH 0x100

static void init_arguments(
  ArgumentParser *parser, const int argc, char *argv[]) {
  parser->add("verbose", 'v');
  parser->add("lang", 'l');
  parser->add("out", 'o');
  parser->add("disassemble", 'd');
  //! deprecated options
  parser->add("compile", 'c');
  parser->add("interpret", 'i');

  parser->parse(argc, argv);
}

// todo remove temp function
void build_path(const File &file, char *path) {
  char dir[MAX_DIR_LENGTH];
  char name[MAX_FILENAME_LENGTH];

  file.get_dir(dir);   // No allocations in get_dir
  file.get_name(name); // No allocations in get_name

  snprintf(path, MAX_DIR_LENGTH + MAX_FILENAME_LENGTH, "%s/%s.hbc", dir, name);
}

static void repl() {
  for (;;) {
    char line[0x400];
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    StringInput input(line);
    Lexer       lexer(input);
    Chunk       chunk;
    Parser      parser_instance(lexer, chunk);

    parse_expression(parser_instance, Precedence::NUL);
    chunk.write(Opcodes::OP_RETURN);

    VM vm;
    vm.interpret(chunk);
  }
}

int main(const int argc, char *argv[]) {
  ArgumentParser parser;

  init_arguments(&parser, argc, argv);

  if (!parser.positional_count) {
    repl();
  }

  for (int i = 0; i < parser.positional_count; i++) {
    File  file(parser.positional_args[i], FILE_MODE_READ);
    Chunk chunk;

    char ext[MAX_EXT_LENGTH];
    file.get_ext(ext);

    if (strncmp(ext, "hbc", 3) == 0) {
      FileHeader header;
      if (file.read_header(&header) != FILE_STATUS_OK) {
        Logger::fatal("Failed to read header");
      }
      char name[MAX_FILENAME_LENGTH];
      if (file.read_name(name, header.name)) {
        Logger::fatal("Failed to read name");
      }

      char c;
      while (file.read_byte(&c) != FILE_READ_DONE) {
        chunk.write(c);
      }
      if (parser.is_set("disassemble")) {
        Logger::disassemble(chunk, name);
        continue;
      }

      VM vm;
      vm.interpret(chunk);
      continue;
    }

    FileInput input(file);
    Lexer     lexer(input);

    Parser parser_instance(lexer, chunk);

    parse_expression(parser_instance, Precedence::NUL);
    chunk.write(Opcodes::OP_RETURN);

    char path[MAX_DIR_LENGTH + MAX_FILENAME_LENGTH];
    build_path(file, path);

    File out(path, FILE_MODE_WRITE);

    if (out.write_header() != FILE_STATUS_OK) {
      Logger::fatal("Failed to write header");
    }

    char name[MAX_FILENAME_LENGTH];
    file.get_name(name);

    out << name;

    for (int j = 0; j < chunk.pos; j++) {
      out << chunk.code[j];
    }
    Logger::disassemble(chunk, name);
  }

  return 0;
}
