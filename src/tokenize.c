#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "utility.h"
#include "config.h"
#include "memory.h"
#include "tokenize.h"
#include "lib/string1.h"

int read_tokens_from_file(int fd, token_t *dest) {
    char *buf = (char *) malloc(EXPR_STR_MAX_LEN);
    int bytes_read = read(fd, buf, EXPR_STR_MAX_LEN);
    if (bytes_read <= -1) {
        perror("read");
        printf("ERROR: TOKENIZER: READ_TOKENS_FROM_FILE: read() returned error\n");
        return -1;
    } else if (bytes_read == 0) {
        printf("INFO: PARSER: EOF\n");
        return EOF_CODE;
    }
    buf[bytes_read] = '\0';

    int res = tokenize(buf, dest);
    if (res < 0) {
        printf("ERROR: TOKENIZER: READ_TOKENS_FROM_FILE: error when tokenizing string\n");
        return res;
    }
    return 0;
}

token_t *tokens_init() {
    token_t *tokens = (token_t *) my_malloc(TOKENS_MAX_NUM * sizeof(token_t));
    for (int i = 0; i < TOKENS_MAX_NUM; i++) {
        tokens[i] = (token_t)NULL;
    }
    return tokens;
}

int tokens_add(token_t *tokens, token_t *new_tokens) {
    for (int i = 0; i < TOKENS_MAX_NUM; i++) {
        if (tokens[i] == NULL) {
            for (int j = 0; j < TOKENS_MAX_NUM; j++) {
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

int tokens_pop(token_t *tokens, int fd, token_t dest) {
    if (tokens == NULL) {
        printf("ERROR: TOKENIZER: TOKENS_POP: tokens was NULL when attempting to pop tokens\n");
        return -1;
    }

    if (tokens[0] == NULL) {
        if (fd == -1) {
            printf("ERROR: TOKENIZER: TOKENS_POP: trying to read more tokens, but fd is -1\n");
            return -1;
        }
        token_t *new_tokens = tokens_init();
        int res = read_tokens_from_file(fd, new_tokens);
        if (res < 0) {
            printf("ERROR: TOKENIZER: TOKENS_POP: reading tokens from file\n");
            return res;
        }
        if (res == EOF_CODE) {
            return EOF_CODE;
        }
        tokens_add(tokens, new_tokens);
    }

    // token_t token = (token_t) my_malloc(TOKEN_STR_MAX_LEN);
    strcpy(dest, tokens[0]);
    int count = 0;
    while (tokens[count] != NULL && tokens[count + 1] != NULL) {
        tokens[count] = tokens[count + 1];
        count++;
    }
    tokens[count] = NULL;

    return 0;
}

int tokens_peek(token_t *tokens, int fd, token_t dest) {
    if (tokens == NULL) {
        printf("ERROR: TOKENIZER: TOKENS_PEEK: tokens was NULL when attempting to peek tokens\n");
        return -1;
    }

    if (tokens[0] == NULL) {
        token_t *new_tokens = tokens_init();
        int res = read_tokens_from_file(fd, new_tokens);
        if (res < 0) {
            printf("ERROR: TOKENIZER: TOKENS_PEEK: Reading tokens from file\n");
            return res;
        }
        if (res == EOF_CODE) {
            return EOF_CODE;
        }
        tokens_add(tokens, new_tokens);
    }
    strcpy(dest, tokens[0]);
    return 0;
}


void tokens_free(token_t *tokens) {
    for(int i = 0; i < TOKENS_MAX_NUM; i++) {
        if (tokens[i] == NULL) {
            break;
        }
        my_free(tokens[i]);
    }
    my_free(tokens);
}

int tokenize(char *src_code, token_t *dest) {
    /* Strip newlines */
    int src_code_size = strlen(src_code), is_in_str = 0;
    for (int i = 0; i < src_code_size; i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == '\n' && !is_in_str) {
            src_code[i] = ' ';
        }
    }

    /* Pad parentheses */
    is_in_str = 0;
    for (int i = 0, j = 0; i < strlen(src_code); i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == ')' || src_code[i] == '(' && !is_in_str) {
            insert_char_in_str(src_code, i + 1, ' ');
            if (insert_char_in_str(src_code, i, ' ') == 0) {
                i++;
            }
        }
    }

    /* Pad quotes */
    is_in_str = 0;
    for (int i = 0, j = 0; i < strlen(src_code); i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == '\'') {
            insert_char_in_str(src_code, i + 1, ' ');
            if (insert_char_in_str(src_code, i, ' ') == 0) {
                i++;
            }
        }
    }

    /* Pad quasiquotes */
    is_in_str = 0;
    for (int i = 0, j = 0; i < strlen(src_code); i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == '`') {
            insert_char_in_str(src_code, i + 1, ' ');
            if (insert_char_in_str(src_code, i, ' ') == 0) {
                i++;
            }
        }
    }

    /* Pad comma-at's */
    is_in_str = 0;
    int found_comma_at = 0;
    for (int i = 0, j = 0; i < strlen(src_code) - 1; i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == ',' && src_code[i + 1] == '@') {
            found_comma_at = 1;
            insert_char_in_str(src_code, i + 2, ' ');
            if (insert_char_in_str(src_code, i, ' ') == 0) {
                i++;
            }
        }
    }

    if (!found_comma_at) {
        /* Pad commas */
        is_in_str = 0;
        for (int i = 0, j = 0; i < strlen(src_code); i++) {
            if (src_code[i] == '"')
                is_in_str = !is_in_str;
            if (src_code[i] == ',') {
                insert_char_in_str(src_code, i + 1, ' ');
                if (insert_char_in_str(src_code, i, ' ') == 0) {
                    i++;
                }
            }
        }
    }

    /* Trim */
    src_code = trim1(src_code, ' ');

    /* Tokenize */
    int num_tokens = 0;
    token_t token = strtok1(src_code, " ");
    while (token != NULL) {
        if (num_tokens >= TOKENS_MAX_NUM) {
            printf("ERROR: TOKENIZER: TOKENIZE: Too many tokens!\n");
            return 01;
        }
        dest[num_tokens++] = token;
        token = strtok1(NULL, " ");
    }

    return 0;
}
