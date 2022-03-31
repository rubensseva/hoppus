#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "parser.h"
#include "utility.h"
#include "config.h"
#include "memory.h"

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

char *tokens_pop(char **tokens) {
    if (tokens[0] == NULL) {
        printf("PARSER: WARNING: No tokens to pop\n");
        return NULL;
    }
    char *token = my_malloc(MAX_TOKEN_LENGTH);
    strcpy(token, tokens[0]);
    int count = 0;
    while (tokens[count] != NULL && tokens[count + 1] != NULL) {
        tokens[count] = tokens[count + 1];
        count++;
    }
    tokens[count] = NULL;
    return token;
}

/**
   Read one list from tokens.

   Returns a expr* tree on success, or NULL on error.
 */
expr *read_from_tokens(char **tokens) {
    char *token = tokens_pop(tokens);

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
        expr *first = NULL;
        expr *curr = NULL;
        expr *prev = NULL;
        while (tokens[0] != NULL && strcmp(")", tokens[0]) != 0) {
            expr *new = read_from_tokens(tokens);
            if (new == NULL) {
                return NULL;
            }
            if (!prev) {
                if (new->type != SYMBOL) {
                    printf("PARSER: ERROR: First entry in a list was not a symbol: %d\n", new->type);
                    return NULL;
                }
            }
            curr = expr_cons(new, NULL);
            if (!first)
                first = curr;
            if (prev)
                prev->cdr = curr;
            prev = curr;
        }
        /* At this points, there should be a ")" on tokens. If thats not the case, we have unmatched
           parentheses. If it is the case, we need to pop it, then continue. */
        if (tokens[0] == NULL) {
            printf("PARSER: ERROR: Unmatched parentheses\n");
            return NULL;
        }
        my_free(tokens_pop(tokens));
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
