#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "utility.h"
#include "config.h"
#include "memory.h"
#include "tokenize.h"

token_t *read_tokens_from_file(int fd) {
    char *buf = (char *) malloc(EXPR_STR_SIZE);
    int bytes_read = read(fd, buf, EXPR_STR_SIZE);
    if (bytes_read == -1) {
        perror("read");
        printf("PARSER: ERROR: Reading from file\n");
        return NULL;
    }
    buf[bytes_read] = '\0';

    token_t *new_tokens = tokenize(buf);
    if (new_tokens == NULL || new_tokens[0] == NULL) {
        printf("PARSER: ERROR: Unable to get additional tokens\n");
        return NULL;
    }
    return new_tokens;
}

token_t *tokens_init() {
    token_t *tokens = (token_t *) my_malloc(MAX_TOKENS * sizeof(token_t));
    for (int i = 0; i < MAX_TOKENS; i++) {
        tokens[i] = (token_t)NULL;
    }
    return tokens;
}

int tokens_add(token_t *tokens, token_t *new_tokens) {
    for (int i = 0; i < MAX_TOKEN_LENGTH; i++) {
        if (tokens[i] == NULL) {
            for (int j = 0; j < MAX_TOKEN_LENGTH; j++) {
                if (new_tokens[j] == NULL) {
                    break;
                }
                tokens[i + j] = new_tokens[j];
            }
            break;
        }
    }
    return 0;
}

token_t tokens_pop(token_t *tokens, int fd) {
    if (tokens == NULL) {
        printf("PARSER: WARNING: Tokens was NULL when attempting to pop tokens\n");
        return NULL;
    }

    if (tokens[0] == NULL) {
        token_t *new_tokens = read_tokens_from_file(fd);
        if (new_tokens == NULL) {
            return NULL;
        }
        tokens_add(tokens, new_tokens);
    }

    token_t token = (token_t) my_malloc(MAX_TOKEN_LENGTH);
    strcpy(token, tokens[0]);
    int count = 0;
    while (tokens[count] != NULL && tokens[count + 1] != NULL) {
        tokens[count] = tokens[count + 1];
        count++;
    }
    tokens[count] = NULL;
    return token;
}

token_t tokens_peek(token_t *tokens, int fd) {
    if (tokens == NULL) {
        printf("PARSER: WARNING: Tokens was NULL when attempting to peek tokens\n");
        return NULL;
    }

    if (tokens[0] == NULL) {
        token_t *new_tokens = read_tokens_from_file(fd);
        if (new_tokens == NULL) {
            return NULL;
        }
        tokens_add(tokens, new_tokens);
    }
    return tokens[0];
}


void tokens_free(token_t *tokens) {
    for(int i = 0; i < MAX_TOKENS; i++) {
        if (tokens[i] == NULL) {
            break;
        }
        my_free(tokens[i]);
    }
    my_free(tokens);
}

token_t *tokenize(char *src_code) {
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
    token_t *tokens = tokens_init();
    int num_tokens = 0;
    token_t token = strtok(src_code, " ");
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
