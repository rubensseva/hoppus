#include <hoppus_utility.h>
#include <hoppus_config.h>
#include <hoppus_memory.h>
#include <hoppus_constants.h>
#include <hoppus_stdio.h>
#include <hoppus_types.h>
#include <tokenize.h>
#include <string1.h>

#include <hoppus_link.h>

#include <stdlib.h>

__USER_TEXT int read_tokens_from_file(int fd, token_t *out) {
    char *buf = (char *) my_malloc(EXPR_STR_MAX_LEN);
    hoppus_read_line(buf);
    // buf[] = '\0';

    int res = tokenize(buf, out);
    if (res < 0) {
        hoppus_puts("ERROR: TOKENIZER: READ_TOKENS_FROM_FILE: error when tokenizing string\n");
        return res;
    }
    return 0;
}

__USER_TEXT token_t *tokens_init() {
    token_t *tokens = (token_t *) my_malloc(TOKENS_MAX_NUM * sizeof(token_t));
    for (int i = 0; i < TOKENS_MAX_NUM; i++) {
        tokens[i] = (token_t)NULL;
    }
    return tokens;
}

__USER_TEXT int tokens_add(token_t *tokens, token_t *new_tokens) {
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

__USER_TEXT int tokens_fill(token_t *tokens, int fd) {
    if (fd == -1) {
        hoppus_puts("ERROR: TOKENIZER: TOKENS_POP: trying to read more tokens, but fd is -1\n");
        return -1;
    }
    token_t *new_tokens = tokens_init();
    int res = read_tokens_from_file(fd, new_tokens);
    if (res < 0) {
        hoppus_puts("ERROR: TOKENIZER: TOKENS_FILL: Reading tokens from file\n");
        return res;
    }
    if (res == EOF_CODE) {
        return EOF_CODE;
    }
    tokens_add(tokens, new_tokens);
    return 0;
}

/**
   @brief Pop the next token from the tokens list, might read more tokens if the list
   is empty.

   @param tokens List of tokens to pop from. Will possibly be refilled if empty.
   Can be prefilled.
   @param fd The file descriptor used to read more tokens. Will not attempt read if
   "fd" is -1.
   @param dest Output parameter. The token will be copied here. If no error occurs,
   it will be malloced with the size of the token.
   @return Return status code */
__USER_TEXT int tokens_pop(token_t *tokens, int fd, token_t *out) {
    int ret_code;
    if (tokens == NULL) {
        hoppus_puts("ERROR: TOKENIZER: TOKENS_POP: tokens was NULL when attempting to pop tokens\n");
        return -1;
    }

    if (tokens[0] == NULL) {
        ret_code = tokens_fill(tokens, fd);
        if (ret_code != 0)
            return ret_code;
    }

    /* We could just return tokens[0], but I have a gut feeling that a malloc and
       strcpy is better. */
    *out = my_malloc(strlen1(tokens[0]) + 1);
    strcpy1(*out, tokens[0]);
    int count = 0;
    while (tokens[count] != NULL && tokens[count + 1] != NULL) {
        tokens[count] = tokens[count + 1];
        count++;
    }
    tokens[count] = NULL;

    return 0;
}

/**
   @brief Peek the next token from the tokens list, might read one more tokens if
   the list is empty.

   @param tokens List of tokens to peek from. Will possibly be refilled if empty.
   Can be prefilled.
   @param fd The file descriptor used to read more tokens. Will not attempt read if
   "fd" is -1.
   @param dest Output parameter. The token will be copied here. If no error occurs,
   it will be malloced with the size of the token.
   @return Return status code */
__USER_TEXT int tokens_peek(token_t *tokens, int fd, token_t *out) {
    int ret_code;
    if (tokens == NULL) {
        hoppus_puts("ERROR: TOKENIZER: TOKENS_PEEK: tokens was NULL when attempting to peek tokens\n");
        return -1;
    }

    if (tokens[0] == NULL) {
        ret_code = tokens_fill(tokens, fd);
        if (ret_code != 0)
            return ret_code;
    }

    *out = my_malloc(strlen1(tokens[0]) + 1);
    strcpy1(*out, tokens[0]);
    return 0;
}

__USER_TEXT int pad_str(char *str, char *pad) {
    int is_in_str = 0;
    unsigned int pad_len = strlen1(pad);
    /* We want to recompute the string length each iteration, since the
       act of padding the string might increase its size */
    for (int i = 0; i < strlen1(str); i++) {
        if (str[i] == '"')
            is_in_str = !is_in_str;

        int pad_match = 1;
        for (int j = 0; j < pad_len && j + i < strlen1(str); j++) {
            if (str[i + j] != pad[j]) {
                pad_match = 0;
                break;
            }
        }
        if (pad_match) {
            insert_char_in_str(str, i + pad_len, ' ');
            if (insert_char_in_str(str, i, ' ') == 0) {
                i++;
            }
        }
    }
    return 0;
}

/**
   @brief Takes a string and tokenizes it. Does not modify "src_code", it makes copies
   of the tokens and puts them in "dest".

   As a reminder, the reason we use constants for strings here is because strings are
   put in the .rodata section by the compiler, which ends up in kernel space. So we need
   some constants that are explicitly put in the user data section.
*/
__USER_TEXT int tokenize(char *src_code, token_t *out) {
    /* Strip newlines */
    int src_code_size = strlen1(src_code), is_in_str = 0;
    for (int i = 0; i < src_code_size; i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == '\n' && !is_in_str) {
            src_code[i] = ' ';
        }
    }

    pad_str(src_code, CLOSING_PAREN);
    pad_str(src_code, OPENING_PAREN);
    pad_str(src_code, QUOTE_SHORT_STR);
    pad_str(src_code, QUASIQUOTE_SHORT_STR);
    pad_str(src_code, COMMA_AT_SHORT_STR);

    /* Pad commas, but not comma-ats */
    is_in_str = 0;
    for (int i = 0, j = 0; i < strlen1(src_code) - 1; i++) {
        if (src_code[i] == '"')
            is_in_str = !is_in_str;
        if (src_code[i] == ',' && src_code[i + 1] != '@') {
            insert_char_in_str(src_code, i + 1, ' ');
            if (insert_char_in_str(src_code, i, ' ') == 0) {
                i++;
            }
        }
    }

    src_code = trim1(src_code, ' ');

    /* Tokenize */
    int num_tokens = 0;
    token_t token = strtok1(src_code, WHITESPACE);
    while (token != NULL) {
        if (num_tokens >= TOKENS_MAX_NUM) {
            hoppus_puts("ERROR: TOKENIZER: TOKENIZE: Too many tokens\n");
            return 01;
        }
        char *new_str = my_malloc(strlen1(token) + 1);
        strcpy1(new_str, token);
        out[num_tokens++] = new_str;
        token = strtok1(NULL, WHITESPACE);
    }

    return 0;
}
