#include <stdio.h>
#include <string.h>

enum tokenizer_state { NORMAL = 0, SIMPLE_QUOTE, DOUBLE_QUOTE, BACKSLASH };
enum command_type { BUILTIN_EXIT, BUILTIN_ECHO, BUILTIN_TYPE, EXECUTABLE, UNKNOWN };

bool in_path(const char *cmd);
const char *tokenize(char *command);
enum command_type get_cmd_type(const char *cmd);
int handle_command(char *command);

int main(int argc, char *argv[]) {
    // Flush after every printf
    setbuf(stdout, NULL);

    while (true) {
        printf("$ ");
        char command[BUFSIZ];
        fgets(command, BUFSIZ, stdin);
        strtok(command, "\n");
        int ret;
        if ((ret = handle_command(command)) == -1) {
            printf("%s: command not found\n", command);
        } else if (ret == 1) {
            break;
        }
    }

    return 0;
}

int handle_command(char *command) {
    const char *token;
    if ((token = tokenize(command)) == nullptr) {
        return 0;
    }
    switch (get_cmd_type(token)) {
    case BUILTIN_EXIT:
        return 1;
    case BUILTIN_ECHO:
        if ((token = tokenize(nullptr)) == nullptr) {
            printf("\n");
            return 0;
        }
        printf("%s", token);
        while ((token = tokenize(nullptr)) != nullptr) {
            printf(" %s", token);
        }
        printf("\n");
        return 0;
    case BUILTIN_TYPE:
        while ((token = tokenize(nullptr)) != nullptr) {
            const char *message;
            switch (get_cmd_type(token)) {
            case BUILTIN_EXIT:
            case BUILTIN_ECHO:
            case BUILTIN_TYPE:
                message = " is a shell builtin";
                break;
            case EXECUTABLE:
                message = " is an executable file";
                break;
            case UNKNOWN:
                message = ": not found";
                break;
            }

            printf("%s%s\n", token, message);
        }
    case EXECUTABLE:
        return 0;
    case UNKNOWN:
    default:
        return -1;
    }
}

const char *tokenize(char *command) {
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

    enum tokenizer_state state = NORMAL;

    while (*read != '\0') {
        switch (state) {
        case NORMAL:
            if (*read == ' ') {
                *write = '\0';
                ++read;
                return token;
            }

            if (*read == '\'') {
                state = SIMPLE_QUOTE;
                ++read;
                continue;
            } else if (*read == '\"') {
                state = DOUBLE_QUOTE;
                ++read;
                continue;
            } else if (*read == '\\') {
                state = BACKSLASH;
            }
            break;
        case SIMPLE_QUOTE:
            if (*read == '\'') {
                state = NORMAL;
                ++read;
                continue;
            }
            break;
        case DOUBLE_QUOTE:
            if (*read == '\"') {
                state = NORMAL;
                ++read;
                continue;
            }
            break;
        case BACKSLASH:
            state = NORMAL;
            break;
        }
        *write = *read;
        ++write;
        ++read;
    }
    *write = '\0';

    return token;
}

enum command_type get_cmd_type(const char *cmd) {
    if (strcmp(cmd, "exit") == 0) {
        return BUILTIN_EXIT;
    }
    if (strcmp(cmd, "echo") == 0) {
        return BUILTIN_ECHO;
    }
    if (strcmp(cmd, "type") == 0) {
        return BUILTIN_TYPE;
    }
    if (in_path(cmd)) {
        return EXECUTABLE;
    }

    return UNKNOWN;
}

bool in_path(const char *cmd) { return false; }
