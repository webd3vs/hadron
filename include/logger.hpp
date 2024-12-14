#ifndef HADRON_LOGGER_H
#define HADRON_LOGGER_H 1

#include "bytecode.hpp"
#include "types.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define LOG_INTERNAL(str, format, ...)                                         \
  do {                                                                         \
    printf("%s", str);                                                         \
    printf(format, ##__VA_ARGS__);                                             \
    printf("\x1b[m\n");                                                        \
  } while (0)

typedef class Logger {
  static bool supportsAnsi() {
    static bool checked   = false;
    static bool supported = false;

    if (checked)
      return supported;
    checked = true;
    if (const char *term = getenv("TERM");
        term && (strstr(term, "xterm") || strstr(term, "color")))
      supported = true;
    return supported;
  }

  public:
  template <typename... Args>
  static void info(const char *format, Args... args) {
    if (supportsAnsi())
      LOG_INTERNAL("\x1b[96mINFO\x1b[m ", format, args...);
    else
      LOG_INTERNAL("INFO: ", format);
  }

  template <typename... Args>
  static void warn(const char *format, Args... args) {
    if (supportsAnsi())
      LOG_INTERNAL("\x1b[93mWARN\x1b[m ", format, args...);
    else
      LOG_INTERNAL("WARN: ", format);
  }

  template <typename... Args>
  static void error(const char *format, Args... args) {
    if (supportsAnsi())
      LOG_INTERNAL("\x1b[91mERROR\x1b[m ", format, args...);
    else
      LOG_INTERNAL("ERROR: ", format);
  }

  template <typename... Args>
  static void fatal(const char *format, Args... args) {
    if (supportsAnsi())
      LOG_INTERNAL("\x1b[91;7;1m FATAL \x1b[0;91m ", format, args...);
    else
      LOG_INTERNAL("FATAL: ", format);
    exit(EXIT_FAILURE);
  }

  static void print_token(const Token &token);
  static void disassemble(const Chunk &chunk, const char *name);
} Logger;

#endif // HADRON_LOGGER_H
