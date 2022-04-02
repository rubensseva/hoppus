#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "expr.h"
#include "parser.h"
#include "utility.h"
#include "config.h"
#include "constants.h"
#include "memory.h"
#include "tokenize.h"


/**
   Create a LISP string representation from a C string.

   The string will be represented as a list, with cons cells,
   where each cell has a char in its ->car field */
expr *expr_from_str(char *str) {
    expr *first = NULL, *prev = NULL;
    unsigned int size = strlen(str);
    int i;
    /* Loop from 2nd element to next to last element, because
       we need to skip the " signs at start and end of the string */
    for(i = 1; i < size - 1; i++) {
        expr *new_char = expr_new(CHAR, (uint64_t)str[i], NULL, NULL);
        expr *new_cons = expr_cons(new_char, NULL);
        if (first == NULL)
            first = new_cons;
        if (prev != NULL)
            prev->cdr = new_cons;
        prev = new_cons;
    }
    /* If empty string, return a single cons cell
       Else, we append an empty cons cell at the end of the list
       to signify the end of the string */
    if (!first)
        return expr_cons(NULL,NULL);
    prev->cdr = expr_cons(NULL, NULL);
    return first;
}


/**
   Read one list from tokens.

   Parameters:
       - tokens: A valid pointer to list of tokens, which this function will fill up.
       - fd: The file descriptor used to fill up tokens

   Returns an expr* tree on success, or NULL on error.
 */
int parse_tokens(token_t *tokens, int fd, expr **res) {
    token_t token = (token_t) my_malloc(MAX_TOKEN_LENGTH);
    int pop_res = tokens_pop(tokens, fd, token);
    if (pop_res < 0) {
        printf("PARSER: ERROR: Error when popping tokens\n");
        return pop_res;
    }
    if (pop_res == EOF_CODE) {
        return EOF_CODE;
    }

    if (is_number(token)) {
        int num = atoi(token);
        expr *new_expr = expr_new(NUMBER, (uint64_t)num, NULL, NULL);
        my_free(token);
        *res = new_expr;
        return 0;
    }

    if (is_boolean(token)) {
        int val;
        if (strcmp(token, BOOL_STR_T) == 0) {
            val = 1;
        } else {
            val = 0;
        }
        expr *new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
        my_free(token);
        *res = new_expr;
        return 0;
    }

    if (is_string(token)) {
        expr *new_expr = expr_from_str(token);
        my_free(token);
        *res = new_expr;
        return 0;
    }

    if (strcmp("(", token) == 0) {
        expr *first = NULL, *curr = NULL, *prev = NULL;
        while (1) {
            token_t peeked_token = (token_t) malloc(MAX_TOKEN_LENGTH);
            int peek_res = tokens_peek(tokens, fd, peeked_token);
            if (peek_res < 0) {
                return peek_res;
            }
            if (peek_res == EOF_CODE) {
                return EOF_WHILE_READING_EXPR_ERROR_CODE;
            }
            if (strcmp(")", peeked_token) == 0) {
                break;
            }

            expr *new;
            int parse_res = parse_tokens(tokens, fd, &new);
            if (parse_res < 0) {
                return parse_res;
            }
            curr = expr_cons(new, NULL);
            if (!first) {
                if (new->type != SYMBOL) {
                    printf("PARSER: ERROR: First entry in a list was not a symbol: %d\n", new->type);
                    return -1;
                }
                first = curr;
            }
            if (prev)
                prev->cdr = curr;
            prev = curr;
        }
        /* At this points, there should be a ")" on tokens, so lets pop it. */
        /* TODO: Maybe we could just use the stack here? */
        token_t closing_paren = (token_t) malloc(MAX_TOKEN_LENGTH);
        int pop_res = tokens_pop(tokens, fd, closing_paren);
        if (pop_res < 0) {
            return pop_res;
        }
        my_free(closing_paren);
        *res = first;
        return 0;
    }

    if (strcmp(")", token) == 0) {
        printf("PARSER: ERROR: Unmatched closing parentheses\n");
        return -1;
    }

    /* If not a number or a parenthesis, then its a symbol */
    expr *new_expr = expr_new(SYMBOL, (uint64_t)token, NULL, NULL);
    *res = new_expr;
    return 0;
}
