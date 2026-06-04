#include <stdio.h>
#include <string.h>

enum tokenizer_state { NORMAL = 0, SIMPLE_QUOTE, DOUBLE_QUOTE, BACKSLASH };

int handle_command(char *command);
const char *tokenize(char *command);

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
    if (strcmp(command, "exit") == 0) {
        return 1;
    } else if (strcmp(token, "echo") == 0) {
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
    }
    return -1;
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
