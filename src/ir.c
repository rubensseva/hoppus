#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ir.h"
#include "utility.h"
#include "config.h"
#include "memory.h"

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
        printf("WARNING: No tokens to pop\n");
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
   Create root node by continually reading from tokens
*/
/* expr *continually_read_from_tokens(char **tokens) { */
/*     expr *first = NULL; */
/*     expr *prev = NULL; */
/*     expr *curr; */
/*     while (tokens[0]) { */
/*         curr = read_from_tokens(tokens); */
/*         if (first == NULL) { */
/*             first = curr; */
/*         } */
/*         if (prev) { */
/*             prev->cdr = curr; */
/*         } */
/*         prev = curr; */
/*     } */
/*     return curr; */
/* } */

/**
   Read one list from tokens
 */
expr *read_from_tokens(char **tokens) {
    char *token = tokens_pop(tokens);

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
        while (strcmp(")", tokens[0]) != 0) {
            expr *new = read_from_tokens(tokens);
            if (!prev) {
                if (new->type != SYMBOL) {
                    printf("Got first entry in a list, but it is not a symbol\n");
                }
            }
            curr = expr_cons(new, NULL);
            if (!first)
                first = curr;
            if (prev)
                prev->cdr = curr;
            prev = curr;
        }
        /* At this point, there is a ")" on tokens[], so we need to pop it */
        my_free(tokens_pop(tokens));
        my_free(token);
        return first;
    }

    /* If not a number or a parenthesis, then its a symbol */
    expr *new_expr = expr_new(SYMBOL, (uint64_t) token, NULL, NULL);
    return new_expr;
}
