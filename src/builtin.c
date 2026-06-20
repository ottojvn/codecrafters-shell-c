#include "builtin.h"
#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct builtin_map {
    const char *cmd;
    builtin_func_t builtin_func;
};

builtin_func_t get_builtin_func(const char *cmd) {
    static const struct builtin_map builtin_cmds[] = {
        {"exit", builtin_exit}, {"echo", echo}, {"type", type}, {"pwd", pwd}};
    for (int i = 0; i < sizeof(builtin_cmds) / sizeof(struct builtin_map); ++i) {
        if (strcmp(cmd, builtin_cmds[i].cmd) == 0) {
            return builtin_cmds[i].builtin_func;
        }
    }

    return NULL;
}

int builtin_exit(const char **argv) { return SHELL_CMD_EXIT; }

int echo(const char **argv) {
    if (*(++argv) != NULL) {
        printf("%s", *argv);
    }
    while (*(++argv) != NULL) {
        printf(" %s", *argv);
    }

    printf("\n");
    return 0;
}

int type(const char **argv) {
    while (*(++argv) != NULL) {
        char full_path[BUFSIZ];
        if (get_builtin_func(*argv) != NULL) {
            printf("%s is a shell builtin\n", *argv);
        } else if (in_path(*argv, full_path)) {
            printf("%s is %s\n", *argv, full_path);
        } else {
            printf("%s: not found\n", *argv);
        }
    }
    return 0;
}

int pwd(const char **argv) {
    char cwd[BUFSIZ];
    if (getcwd(cwd, BUFSIZ) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }
    printf("%s\n", cwd);
    return EXIT_SUCCESS;
}
