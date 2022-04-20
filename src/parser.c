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
   @brief Create a LISP string representation from a C string.

   The string will be represented as a list, with cons cells,
   where each cell has a char in its ->car field

   The data of the string is copied.
*/
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
    return first;
}

/**
   @brief Parse one of the symbols that deals with quoation, such as quote,
   quasiquote, comma and comma-at. */
int parse_quotation_symbol(token_t *tokens, int fd, char *name, expr **out) {
    int ret_code; expr *parsed;
    if ((ret_code = parse_tokens(tokens, fd, &parsed)) < 0)
        return ret_code;
    *out = expr_cons(expr_new(SYMBOL, (uint64_t)name, NULL, NULL),
                     expr_cons(parsed, NULL));
    return 0;
}


/**
   @brief Continually parse tokens until a complete LISP expression is parsed

   Calls "tokens_pop" to fetch new tokens.

   @param tokens A pointer to a list of tokens. This list could be prefilled
                 with tokens, or it may be empty, in which case a read from
                 file might be attempted to fill it up.
   @param fd The file descriptor that will be used to fill up tokens. Can be
             set to -1 to indicate that tokens should not be refilled.

   @return Return status code. Will be less than 0 on error
 */
int parse_tokens(token_t *tokens, int fd, expr **out) {
    int ret_code; token_t token;
    ret_code = tokens_pop(tokens, fd, &token);
    if (ret_code < 0) {
        printf("ERROR: PARSER: popping tokens\n");
        return ret_code;
    }
    if (ret_code == EOF_CODE)
        return EOF_CODE;

    if (strcmp(token, QUOTE_SHORT_STR) == 0) {
        if ((ret_code = parse_quotation_symbol(tokens, fd, QUOTE_STR, out)) < 0)
            return ret_code;
    } else if (strcmp(token, QUASIQUOTE_SHORT_STR) == 0) {
        if ((ret_code = parse_quotation_symbol(tokens, fd, QUASIQUOTE_STR, out)) < 0)
            return ret_code;
    } else if (strcmp(token, COMMA_SHORT_STR) == 0) {
        if ((ret_code = parse_quotation_symbol(tokens, fd, COMMA_STR, out) < 0))
            return ret_code;
    } else if (strcmp(token, COMMA_AT_SHORT_STR) == 0) {
        if ((ret_code = parse_quotation_symbol(tokens, fd, COMMA_AT_STR, out)) < 0)
            return ret_code;
    } else if (is_number(token)) {
        *out = expr_new(NUMBER, (uint64_t)atoi(token), NULL, NULL);
    } else if (is_boolean(token)) {
        int val = strcmp(token, BOOL_STR_T) == 0 ? 1 : 0;
        *out = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    } else if (is_string(token)) {
        *out = expr_from_str(token);
    } else if (is_nil(token)) {
        *out = NULL;
    } else if (strcmp("(", token) == 0) {
        expr *first = NULL, *curr = NULL, *prev = NULL;
        while (1) {
            token_t peeked_token;
            if ((ret_code = tokens_peek(tokens, fd, &peeked_token) < 0))
                return ret_code;
            if (ret_code == EOF_CODE)
                return EOF_WHILE_READING_EXPR_ERROR_CODE;
            int cmp = strcmp(")", peeked_token) == 0;
            if (cmp) break;

            expr *new;
            if ((ret_code = parse_tokens(tokens, fd, &new)) < 0)
                return ret_code;
            if (ret_code == EOF_CODE)
                return EOF_WHILE_READING_EXPR_ERROR_CODE;
            curr = expr_cons(new, NULL);
            if (!first) {
                if (new->type != SYMBOL)
                    printf("WARNING: PARSER: First entry in a list was not a symbol, instead it had type: %d\n", new->type);
                first = curr;
            }
            if (prev)
                prev->cdr = curr;
            prev = curr;
        }
        /* At this points, there should be a ")" on tokens, so lets pop it. */
        token_t closing_paren;
        if ((ret_code = tokens_pop(tokens, fd, &closing_paren) < 0))
            return ret_code;
        *out = first;
    } else if (strcmp(")", token) == 0) {
        printf("ERROR: PARSER: Unmatched closing parentheses\n");
        return -1;
    } else {
        /* If not any of the above, then its a symbol */
        token_t symbol_name = my_malloc(strlen(token) + 1);
        strcpy(symbol_name, token);
        *out = expr_new(SYMBOL, (uint64_t)symbol_name, NULL, NULL);
    }

    return 0;
}
