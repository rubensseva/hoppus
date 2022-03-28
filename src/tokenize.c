#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utility.h"
#include "config.h"


int insert_char_in_str(char *str, int i, char c) {
    int str_size = strlen(str);
    if (i < 0 || i == str_size) {
        return -1;
    }
    for (int j = str_size; j > i; j--) {
        str[j] = str[j - 1];
    }
    str[str_size + 1] = '\0';
    str[i] = c;
    return 0;
}

void free_tokens(char **tokens) {
    for(int i = 0; i < MAX_TOKENS; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

char **tokenize(char *src_code) {
    /* Strip newlines */
    int src_code_size = strlen(src_code);
    for (int i = 0; i < src_code_size; i++) {
        if (src_code[i] == '\n') {
            src_code[i] = ' ';
        }
    }

    /* Pad parentheses */
    for (int i = 0, j = 0; i < strlen(src_code); i++) {
        if (src_code[i] == ')' || src_code[i] == '(') {
            insert_char_in_str(src_code, i + 1, ' ');
            if (insert_char_in_str(src_code, i, ' ') == 0) {
                i++;
            }
        }
    }

    /* Tokenize */
    char **tokens = (char **) malloc(MAX_TOKENS * sizeof(char *));
    for (int i = 0; i < MAX_TOKENS; i++) {
        tokens[i] = (char *)NULL;
    }
    int num_tokens = 0;
    char *token = strtok(src_code, " ");
    while (token != NULL) {
        if (num_tokens >= MAX_TOKENS) {
            printf("ERROR: Too many tokens!\n");
            return NULL;
        }
        tokens[num_tokens++] = token;
        token = strtok(NULL, " ");
    }

    return tokens;
}
