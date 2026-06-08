#include <stdio.h>
#include <string.h>

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

static bool in_path(const char *cmd);
static const char *tokenize(char *command);
static enum command_type get_cmd_type(const char *cmd);
static int handle_command(char *command);
static int echo(const char *arg);
static int type(const char *arg);

int main(int argc, char *argv[]) {
    // Flush after every printf
    setbuf(stdout, NULL);

    while (true) {
        printf("$ ");
        char input_line[BUFSIZ];
        if (fgets(input_line, BUFSIZ, stdin) == NULL) {
            break;
        }
        input_line[strcspn(input_line, "\n")] = '\0';
        int ret;
        if ((ret = handle_command(input_line)) == -1) {
            printf("%s: command not found\n", input_line);
        } else if (ret == 1) {
            break;
        }
    }

    return 0;
}

static int handle_command(char *command) {
    const char *token;
    if ((token = tokenize(command)) == nullptr) {
        return 0;
    }
    switch (get_cmd_type(token)) {
    case CMD_BUILTIN_EXIT:
        return 1;
    case CMD_BUILTIN_ECHO:
        return echo(token);
    case CMD_BUILTIN_TYPE:
        return type(token);
    case CMD_EXECUTABLE:
        return 0;
    case CMD_UNKNOWN:
    default:
        return -1;
    }
}

static const char *tokenize(char *command) {
    static char *read = nullptr;
    static char *token = nullptr;
    static char *write = nullptr;
    // novo command
    if (command != nullptr) {
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
        return nullptr;
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
            } else if (*read == '\"') {
                state = STATE_DOUBLE_QUOTE;
                ++read;
                continue;
            } else if (*read == '\\') {
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

static enum command_type get_cmd_type(const char *cmd) {
    static const struct builtin_map builtin_cmds[] = {
        {"exit", CMD_BUILTIN_EXIT}, {"echo", CMD_BUILTIN_ECHO}, {"type", CMD_BUILTIN_TYPE}};
    for (int i = 0; i < sizeof(builtin_cmds) / sizeof(struct builtin_map); ++i) {
        if (strcmp(cmd, builtin_cmds[i].cmd) == 0) {
            return builtin_cmds[i].type;
        }
    }
    if (in_path(cmd)) {
        return CMD_EXECUTABLE;
    }

    return CMD_UNKNOWN;
}

static bool in_path(const char *cmd) { return false; }

static int echo(const char *arg) {
    if ((arg = tokenize(nullptr)) == nullptr) {
        printf("\n");
        return 0;
    }
    printf("%s", arg);
    while ((arg = tokenize(nullptr)) != nullptr) {
        printf(" %s", arg);
    }
    printf("\n");
    return 0;
}

static int type(const char *arg) {
    while ((arg = tokenize(nullptr)) != nullptr) {
        const char *message;
        switch (get_cmd_type(arg)) {
        case CMD_BUILTIN_EXIT:
        case CMD_BUILTIN_ECHO:
        case CMD_BUILTIN_TYPE:
            message = " is a shell builtin";
            break;
        case CMD_EXECUTABLE:
            message = " is an executable file";
            break;
        case CMD_UNKNOWN:
            message = ": not found";
            break;
        }

        printf("%s%s\n", arg, message);
    }
    return 0;
}
