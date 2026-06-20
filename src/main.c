#include "builtin.h"
#include "parse.h"
#include "process.h"
#include <stdio.h>
#include <string.h>

static int handle_cmd(char *cmd);

int main(void) {
    // Flush after every printf
    setbuf(stdout, NULL);

    while (true) {
        printf("$ ");
        char input_line[BUFSIZ];
        if (fgets(input_line, BUFSIZ, stdin) == NULL) {
            break;
        }
        input_line[strcspn(input_line, "\n")] = '\0';
        int status = handle_cmd(input_line);
        if (status == SHELL_CMD_EXIT) {
            break;
        }
        if (status == -1) {
            printf("%s: command not found\n", input_line);
        }
    }

    return 0;
}

static int handle_cmd(char *cmd) {
    char *argv[4096] = {NULL};
    int i = 0;
    char *saveptr = NULL;
    *argv = tokenize(cmd, &saveptr);
    while (*argv != NULL && i < 4095) {
        i++;
        argv[i] = tokenize(NULL, &saveptr);
    }
    if (*argv == NULL) {
        return 0;
    }
    char full_path[BUFSIZ];
    builtin_func_t func = get_builtin_func(*argv);
    if (func != NULL) {
        return func((const char **)argv);
    }
    if (in_path(argv[0], full_path)) {
        return execute(full_path, argv);
    }

    return -1;
}
