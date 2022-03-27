#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ir.h"
#include "utility.h"

char *tokens_pop(char **tokens) {
    char *token = malloc(256);
    strcpy(token, tokens[0]);
    int count = 0;
    while (tokens[count] != NULL && tokens[count + 1] != NULL) {
        tokens[count] = tokens[count + 1];
        count++;
    }
    return token;
}

expr *read_from_tokens(char **tokens) {
    char *token = tokens_pop(tokens);
    if (is_number(token)) {
        int num = atoi(token);
        expr *new_expr = (expr *)malloc(sizeof(expr));
        new_expr->type = NUMBER;
        new_expr->data = num;
        new_expr->next = NULL;
        return new_expr;
    }

    if (strcmp("(", token) == 0) {
        expr *list_expr = (expr *)malloc(sizeof(expr));
        list_expr->type = LIST;
        list_expr->data = 0;
        list_expr->next = NULL;

        expr *prev = NULL;

        while (strcmp(")", tokens[0]) != 0) {
            expr *new = read_from_tokens(tokens);
            /* Let curr->data point to the first entry in the list */
            if (!list_expr->data) {
                list_expr->data = (uint64_t)new;
            }
            if (prev) {
                prev->next = new;
            } else {
                if (new->type != SYMBOL) {
                    printf("Got first entry in a list, but it is not a symbol\n");
                }
                new->type = PROC;
            }
            prev = new;
        }
        return list_expr;
    }

    /* If not a number or a parenthesis, then its a symbol */
    expr *new_expr = (expr *)malloc(sizeof(expr));
    new_expr->type = SYMBOL;
    new_expr->data = (uint64_t) token;
    new_expr->next = NULL;
    return new_expr;
}
