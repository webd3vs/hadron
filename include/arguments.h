#ifndef HADRON_ARGUMENTS_H
#define HADRON_ARGUMENTS_H 1

#define MAX_ARGS 10

typedef struct Argument {
  const char *long_name;
  char        short_name;
  char       *value;
  bool        is_set;
  bool        has_value;
} Argument;

typedef class ArgumentParser {
  Argument &def();
  Argument &find(const char *name);
  Argument &find_short(char name);

  public:
  int      arg_count{0};
  int      positional_count{0};
  Argument args[MAX_ARGS]{};
  char    *positional_args[MAX_ARGS]{};

  void  add(const char *long_name, char short_name);
  void  parse(int argc, char *argv[]);
  char *get(const char *name);
  bool  is_set(const char *name);

  ArgumentParser() {
    add(nullptr, '\0');
    args[0].has_value = false;
  }
} ArgumentParser;

#endif // HADRON_ARGUMENTS_H
