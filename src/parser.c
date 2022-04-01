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
expr *parse_tokens(token_t *tokens, int fd) {
    token_t token = tokens_pop(tokens, fd);
    if (token == NULL) {
        printf("PARSER: ERROR: Token was NULL\n");
        return NULL;
    }

    if (is_number(token)) {
        int num = atoi(token);
        expr *new_expr = expr_new(NUMBER, num, NULL, NULL);
        my_free(token);
        return new_expr;
    }

    if (strcmp("(", token) == 0) {
        expr *first = NULL, *curr = NULL, *prev = NULL;
        while (tokens_peek(tokens, fd) == NULL || strcmp(")", tokens_peek(tokens, fd)) != 0) {
            expr *new = parse_tokens(tokens, fd);
            if (new == NULL) {
                return NULL;
            }
            curr = expr_cons(new, NULL);
            if (!first) {
                if (new->type != SYMBOL) {
                    printf("PARSER: ERROR: First entry in a list was not a symbol: %d\n", new->type);
                    return NULL;
                }
                first = curr;
            }
            if (prev)
                prev->cdr = curr;
            prev = curr;
        }
        /* At this points, there should be a ")" on tokens, so lets pop it. */
        my_free(tokens_pop(tokens, fd));
        my_free(token);
        return first;
    }

    if (strcmp(")", token) == 0) {
        printf("PARSER: ERROR: Unmatched closing parentheses\n");
        return NULL;
    }

    /* If not a number or a parenthesis, then its a symbol */
    expr *new_expr = expr_new(SYMBOL, (uint64_t) token, NULL, NULL);
    return new_expr;
}
