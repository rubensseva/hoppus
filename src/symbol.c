#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "config.h"
#include "memory.h"

int n_symbol = 0;
symbol *symbols[MAX_SYMBOLS];

void symbol_add(symbol *s) {
    if (n_symbol >= MAX_SYMBOLS - 1) {
        printf("ERROR: Too many symbols\n");
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
    return 1;
}
