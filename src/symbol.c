#include <string.h>
#include <stdlib.h>

#include "symbol.h"

int n_symbol = 0;
symbol *symbols[1024];

void symbol_add(symbol *s) {
    symbols[n_symbol++] = s;
}

symbol *symbol_find(char *s) {
    for (int i = 0; i < n_symbol; i++) {
        if (strcmp(symbols[i]->name, s) == 0) {
            return symbols[i];
        }
    }
    return NULL;
}

int symbol_remove_i(int index) {
    free(symbols[index]);
    for (int i = index; i < n_symbol; i++) {
        symbols[i] = symbols[i + 1];
    }
    n_symbol--;

    return 0;
}

int symbol_remove_name(char *s) {
    for (int i = 0; i < n_symbol; i++) {
        if (strcmp(symbols[i]->name, s) == 0) {
            return symbol_remove_i(i);
        }
    }
    return 1;
}
