#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef _WIN32
const char *PATH_LIST_DELIMITER = ";";
const char *DIR_DELIMITER = "\\";
#else
const char *PATH_LIST_DELIMITER = ":";
const char *DIR_DELIMITER = "/";
#endif

enum tokenizer_state { STATE_NORMAL = 0, STATE_SIMPLE_QUOTE, STATE_DOUBLE_QUOTE, STATE_BACKSLASH };
enum command_type {
    CMD_BUILTIN_EXIT,
    CMD_BUILTIN_ECHO,
    CMD_BUILTIN_TYPE,
    CMD_EXECUTABLE,
    CMD_UNKNOWN
};

struct builtin_map {
    const char *cmd;
    enum command_type type;
};

static bool in_path(const char *cmd, char *full_path);
static char *tokenize(char *command);
static enum command_type get_cmd_type(const char *cmd, char *full_path);
static int handle_command(char *command);
static int echo(const char *arg);
static int type(const char *cmd);
static int execute(const char *cmd_path, char *arg0);

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
        int ret = handle_command(input_line);
        if (ret == -1) {
            printf("%s: command not found\n", input_line);
        } else if (ret == 1) {
            break;
        }
    }

    return 0;
}

static int handle_command(char *command) {
    const char *token = tokenize(command);
    if (token == NULL) {
        return 0;
    }
    char full_path[BUFSIZ];
    switch (get_cmd_type(token, full_path)) {
    case CMD_BUILTIN_EXIT:
        return 1;
    case CMD_BUILTIN_ECHO:
        return echo(token);
    case CMD_BUILTIN_TYPE:
        return type(token);
    case CMD_EXECUTABLE:
        return execute(full_path, (char *)token);
    case CMD_UNKNOWN:
    default:
        return -1;
    }
}

static char *tokenize(char *command) {
    static char *read = NULL;
    static char *token = NULL;
    static char *write = NULL;
    // novo command
    if (command != NULL) {
        read = command;
        token = command;
    } else { // mesmo command anterior
        while (*read == ' ') {
            ++read;
        }
        token = read;
    }
    write = token;

    // fim do command
    if (*token == '\0') {
        return NULL;
    }

    enum tokenizer_state state = STATE_NORMAL;

    while (*read != '\0') {
        switch (state) {
        case STATE_NORMAL:
            if (*read == ' ') {
                *write = '\0';
                ++read;
                return token;
            }

            if (*read == '\'') {
                state = STATE_SIMPLE_QUOTE;
                ++read;
                continue;
            }
            if (*read == '\"') {
                state = STATE_DOUBLE_QUOTE;
                ++read;
                continue;
            }
            if (*read == '\\') {
                state = STATE_BACKSLASH;
            }
            break;
        case STATE_SIMPLE_QUOTE:
            if (*read == '\'') {
                state = STATE_NORMAL;
                ++read;
                continue;
            }
            break;
        case STATE_DOUBLE_QUOTE:
            if (*read == '\"') {
                state = STATE_NORMAL;
                ++read;
                continue;
            }
            break;
        case STATE_BACKSLASH:
            state = STATE_NORMAL;
            break;
        }
        *write = *read;
        ++write;
        ++read;
    }
    *write = '\0';

    return token;
}

static enum command_type get_cmd_type(const char *cmd, char *full_path) {
    static const struct builtin_map builtin_cmds[] = {
        {"exit", CMD_BUILTIN_EXIT}, {"echo", CMD_BUILTIN_ECHO}, {"type", CMD_BUILTIN_TYPE}};
    for (int i = 0; i < sizeof(builtin_cmds) / sizeof(struct builtin_map); ++i) {
        if (strcmp(cmd, builtin_cmds[i].cmd) == 0) {
            return builtin_cmds[i].type;
        }
    }
    if (in_path(cmd, full_path)) {
        return CMD_EXECUTABLE;
    }

    return CMD_UNKNOWN;
}

static bool in_dir(const char *dir_path, const char *cmd, char *full_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        return false;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip current and parent (. and ..)
        if (strcmp(entry->d_name, cmd) != 0) {
            continue;
        }

        if (snprintf(full_path, BUFSIZ, "%s%s%s", dir_path, DIR_DELIMITER, cmd) < 0) {
            perror("snprintf");
            continue;
        }

        // get file information
        struct stat file_stat;
        if (stat(full_path, &file_stat) == 0 &&                            // got file stat
            (S_ISREG(file_stat.st_mode) && file_stat.st_mode & S_IXUSR)) { // is executable
            closedir(dir);
            return true;
        }
    }
    closedir(dir);
    return false;
}

static bool in_path(const char *cmd, char *full_path) {
    char *path_env = getenv("PATH");
    char path[BUFSIZ];
    strncpy(path, path_env, BUFSIZ);
    const char *dir_path = NULL;
    dir_path = strtok(path, PATH_LIST_DELIMITER);
    if (dir_path != NULL && (int)(in_dir(dir_path, cmd, full_path))) {
        return true;
    }
    while ((dir_path = strtok(NULL, PATH_LIST_DELIMITER)) != NULL) {
        if (in_dir(dir_path, cmd, full_path)) {
            return true;
        }
    }
    return false;
}

static int echo(const char *arg) {
    arg = tokenize(NULL);
    if (arg == NULL) {
        printf("\n");
        return 0;
    }
    printf("%s", arg);
    while ((arg = tokenize(NULL)) != NULL) {
        printf(" %s", arg);
    }
    printf("\n");
    return 0;
}

static int type(const char *cmd) {
    while ((cmd = tokenize(NULL)) != NULL) {
        char full_path[BUFSIZ];
        switch (get_cmd_type(cmd, full_path)) {
        case CMD_BUILTIN_EXIT:
        case CMD_BUILTIN_ECHO:
        case CMD_BUILTIN_TYPE:
            printf("%s is a shell builtin\n", cmd);
            break;
        case CMD_EXECUTABLE:
            printf("%s is %s\n", cmd, full_path);
            break;
        case CMD_UNKNOWN:
            printf("%s: not found\n", cmd);
            break;
        }
    }
    return 0;
}

static int get_args(char **args) {
    int i = 1;
    while ((args[i++] = tokenize(NULL)) != NULL) {
        ;
    }

    return 0;
}

static int execute(const char *cmd_path, char *arg0) {
    pid_t child = fork();
    char *args[4096];
    args[0] = arg0;
    switch (child) {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        get_args(args);
        execvp(cmd_path, (char *const *)args);
        if (fflush(stdout) < 0) {
            perror("fflush");
        }
        _exit(EXIT_SUCCESS);
    default:
        waitpid(child, NULL, 0);
        return 0;
    }
}
