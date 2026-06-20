#ifndef BUILTIN_H_
#define BUILTIN_H_

#define SHELL_CMD_EXIT -255

typedef int (*builtin_func_t)(const char **argv);

builtin_func_t get_builtin_func(const char *cmd);

int builtin_exit(const char **argv);
int echo(const char **argv);
int type(const char **argv);
int pwd(const char **argv);

#endif // BUILTIN_H_
