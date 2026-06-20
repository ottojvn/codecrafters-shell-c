#include "parse.h"
#include <stddef.h>

enum tokenizer_state { STATE_NORMAL = 0, STATE_SIMPLE_QUOTE, STATE_DOUBLE_QUOTE, STATE_BACKSLASH };

char *tokenize(char *cmd, char **saveptr) {
    char *in_ptr = NULL;
    char *token = NULL;
    char *out_ptr = NULL;
    // novo cmd
    if (cmd != NULL) {
        *saveptr = cmd;
    }
    in_ptr = *saveptr;
    while (*in_ptr == ' ') {
        ++in_ptr;
    }
    token = in_ptr;
    out_ptr = token;

    // fim do cmd
    if (*token == '\0') {
        return NULL;
    }

    enum tokenizer_state state = STATE_NORMAL;
    enum tokenizer_state prev_state = state;

    while (*in_ptr != '\0') {
        switch (state) {
        case STATE_NORMAL:
            if (*in_ptr == ' ') {
                *out_ptr = '\0';
                ++in_ptr;
                *saveptr = in_ptr;
                return token;
            }
            if (*in_ptr == '\'') {
                state = STATE_SIMPLE_QUOTE;
                ++in_ptr;
                continue;
            }
            if (*in_ptr == '\\') {
                prev_state = state;
                state = STATE_BACKSLASH;
                ++in_ptr;
                continue;
            }
            if (*in_ptr == '\"') {
                state = STATE_DOUBLE_QUOTE;
                ++in_ptr;
                continue;
            }
            break;
        case STATE_SIMPLE_QUOTE:
            if (*in_ptr == '\'') {
                state = STATE_NORMAL;
                ++in_ptr;
                continue;
            }
            break;
        case STATE_DOUBLE_QUOTE:
            if (*in_ptr == '\\') {
                prev_state = state;
                state = STATE_BACKSLASH;
                ++in_ptr;
                continue;
            }
            if (*in_ptr == '\"') {
                state = STATE_NORMAL;
                ++in_ptr;
                continue;
            }
            break;
        case STATE_BACKSLASH:
            state = prev_state;
            break;
        }
        *out_ptr = *in_ptr;
        ++out_ptr;
        ++in_ptr;
    }
    *out_ptr = '\0';

    *saveptr = in_ptr;
    return token;
}
