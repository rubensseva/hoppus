#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_TOKENS 1024

int insert_char_in_str(char *str, int i, char c) {
    if (i < 0 || i == strlen(str)) {
        return -1;
    }
    for (int j = strlen(str); j > i; j--) { // need + 1 to include null byte
        str[j] = str[j - 1];
    }
    str[i] = c;
    return 0;
}



char **tokenize(char *src_code) {
    /* Strip newlines */
    for (int i = 0, j = 0; i < strlen(src_code); i++) {
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
    char **tokens = (char **)malloc(MAX_TOKENS);
    int num_tokens = 0;
    char *token = strtok(src_code, " ");
    while (token != NULL) {
        tokens[num_tokens++] = token;
        token = strtok(NULL, " ");
    }

    return tokens;
}
