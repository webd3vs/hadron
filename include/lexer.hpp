#ifndef HADRON_LEXER_H
#define HADRON_LEXER_H 1

#include "types.hpp"

#include <input.hpp>

class Lexer {
  bool         end{false};
  char         current_char{'\0'};
  char         next_char{'\0'};
  int          iterator{-1};
  int          character{1};
  int          line{1};
  int          start{0};
  int          absStart{0};
  InputSource &input;

  char               next();
  [[nodiscard]] char current() const;
  [[nodiscard]] char peek() const;
  [[nodiscard]] char peek2() const;
  [[nodiscard]] bool match(char c);

  [[nodiscard]] Token emit(Type type) const;
  [[nodiscard]] Token emit(Type type, double value) const;
  [[nodiscard]] Token number(char first_char);

  public:
  explicit Lexer(InputSource &input) : input(input) {
    next_char = input.next(); // manually load the first char
  }
  Token advance();
};

#endif // HADRON_LEXER_H
