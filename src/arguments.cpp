#include "arguments.hpp"
#include "logger.hpp"

#include <cstring>

void ArgumentParser::add(const char *long_name, const char short_name) {
  args[arg_count].long_name  = long_name;
  args[arg_count].short_name = short_name;
  args[arg_count].has_value  = true;
  arg_count++;
}

Argument &ArgumentParser::def() { return args[0]; }

Argument &ArgumentParser::find(const char *name) {
  for (int i = 0; i < arg_count; i++) {
    if (args[i].long_name && strcmp(args[i].long_name, name) == 0) {
      return args[i];
    }
  }
  return def();
}

Argument &ArgumentParser::find_short(const char name) {
  for (int i = 0; i < arg_count; i++) {
    if (args[i].short_name == name) {
      return args[i];
    }
  }
  return def();
}

void ArgumentParser::parse(const int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      // scan positional argument
      positional_args[positional_count++] = argv[i];
    } else if (argv[i][1] != '-') {
      // scan short argument
      const char short_name = argv[i][1];
      Argument  &arg        = find_short(short_name);
      if (!arg.has_value) {
        Logger::warn("Unknown argument");
        continue;
      }
      arg.is_set = true;
      if (argv[i][2] == '=') {
        arg.value = argv[i] + 3;
      } else if (argv[i][2] == '\0' && arg.has_value) {
        arg.value = argv[++i];
      } else if (arg.has_value) {
        arg.value = argv[i] + 2;
      }
    } else {
      // scan long argument
      const char *long_name = argv[i] + 2;
      char       *value     = strchr(argv[i] + 2, '=');
      if (value) {
        *value++ = '\0';
      }
      Argument &arg = find(long_name);
      if (!arg.has_value) {
        Logger::warn("Unknown argument");
        continue;
      }
      arg.is_set = true;
      if (!value) {
        if (i == argc - 1) {
          Logger::fatal("Unexpected end of arguments");
          return;
        }
        arg.value = argv[++i];
      } else {
        arg.value = value;
      }
    }
  }
}

char *ArgumentParser::get(const char *name) { return find(name).value; }

bool ArgumentParser::is_set(const char *name) { return find(name).is_set; }
