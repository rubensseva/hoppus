#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "config.h"
#include "memory.h"

int n_symbol = 0;
symbol *symbols[SYMBOLS_MAX_NUM];

symbol *__symbol_create(char *name, symbol_type type, expr *e, builtin_fn_t *fn, int is_special_operator) {
    symbol *new = (symbol *)malloc(sizeof(symbol));
    new->name = name;
    new->type = type;
    new->e = e;
    new->builtin_fn = fn;
    new->is_special_operator = is_special_operator;
    return new;
}
symbol *symbol_create(char *name, symbol_type type, expr *e) {
    return __symbol_create(name, type, e, NULL, 0);
}
symbol *symbol_builtin_create(char *name, builtin_fn_t *fn, int is_special_operator) {
    return __symbol_create(name, BUILTIN, NULL, fn, is_special_operator);
}

void symbol_add(symbol *s) {
    if (n_symbol >= SYMBOLS_MAX_NUM - 1) {
        printf("ERROR: SYMBOL: Too many symbols\n");
    } else {
        symbols[n_symbol++] = s;
    }
}

symbol *symbol_find(char *s) {
    /* Iterate from top to bottom to implement shadowed variables.
       Symbols will work in a stack-ish kind of way when multiple variables
       with the same name are defined */
    for (int i = n_symbol - 1; i >= 0; i--) {
        if (strcmp(symbols[i]->name, s) == 0) {
            return symbols[i];
        }
    }
    return NULL;
}

int symbol_remove_i(int index) {
    my_free(symbols[index]);
    for (int i = index; i < n_symbol; i++) {
        symbols[i] = symbols[i + 1];
    }
    n_symbol--;

    return 0;
}

int symbol_remove_name(char *s) {
    /* Iterate from top to bottom to implement shadowed variables.
       Symbols will work in a stack-ish kind of way when multiple variables
       with the same name are defined */
    for (int i = n_symbol - 1; i >= 0; i--) {
        if (strcmp(symbols[i]->name, s) == 0) {
            return symbol_remove_i(i);
        }
    }
    return -1;
}
