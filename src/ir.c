#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ir.h"
#include "utility.h"
#include "config.h"

char *tokens_pop(char **tokens) {
    if (tokens[0] == NULL) {
        printf("WARNING: No tokens to pop\n");
        return NULL;
    }
    char *token = malloc(MAX_TOKEN_LENGTH);
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
expr *continually_read_from_tokens(char **tokens) {
    expr *root_expr = (expr *)malloc(sizeof(expr));
    root_expr->type = ROOT;
    root_expr->data = 0;
    root_expr->next = NULL;

    expr *prev_holder;
    while (tokens[0]) {
        expr *curr_expr = read_from_tokens(tokens);
        expr *holder = malloc(sizeof(expr));
        holder->type = SEQUENCE_HOLDER;
        holder->data = (uint64_t) curr_expr;
        holder->next = NULL;
        if (root_expr->data == 0) {
            root_expr->data = (uint64_t) holder;
        }
        if (prev_holder) {
            prev_holder->next = holder;
        }
        prev_holder = holder;
    }
    return root_expr;
}

/**
   Read one list from tokens
 */
expr *read_from_tokens(char **tokens) {
    char *token = tokens_pop(tokens);

    if (is_number(token)) {
        int num = atoi(token);
        expr *new_expr = (expr *)malloc(sizeof(expr));
        new_expr->type = NUMBER;
        new_expr->data = num;
        new_expr->next = NULL;
        free(token);
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
                new->type = PROC_SYMBOL;
            }
            prev = new;
        }
        /* At this point, there is a ")" on tokens[], so we need to pop it */
        free(tokens_pop(tokens));
        free(token);
        return list_expr;
    }

    /* If not a number or a parenthesis, then its a symbol */
    expr *new_expr = (expr *)malloc(sizeof(expr));
    new_expr->type = SYMBOL;
    new_expr->data = (uint64_t) token;
    new_expr->next = NULL;
    return new_expr;
}
