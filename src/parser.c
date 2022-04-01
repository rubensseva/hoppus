#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "parser.h"
#include "utility.h"
#include "config.h"
#include "memory.h"
#include "tokenize.h"

char *unknown_type_str = "unknown";
char *expr_type_string_map[EXPR_TYPE_ENUM_SIZE] = {
    "symbol",
    "number",
    "cons"
};

char *type_str(expr_type tp) {
    if (tp < 0 || tp > EXPR_TYPE_ENUM_SIZE) {
        return unknown_type_str;
    }
    return expr_type_string_map[tp];
}

expr *expr_new(expr_type type, uint64_t data, expr* car, expr *cdr) {
    expr *new = (expr *)my_malloc(sizeof(expr));
    new->type = type;
    new->data = data;
    new->car = car;
    new->cdr = cdr;
    return new;
}
expr *expr_copy(expr* src) {
    return expr_new(src->type, src->data, src->car, src->cdr);
}
expr *expr_cons(expr* car, expr *cdr) {
    return expr_new(CONS, 0, car, cdr);
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
        expr *new_expr = expr_new(NUMBER, num, NULL, NULL);
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
    expr *new_expr = expr_new(SYMBOL, (uint64_t) token, NULL, NULL);
    *res = new_expr;
    return 0;
}
