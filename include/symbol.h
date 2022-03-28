#ifndef SYMBOL_H_
#define SYMBOL_H_

#include "ir.h"

typedef struct symbol_t {
    char *name;
    expr *e;
} symbol;

void symbol_add(symbol *s);
symbol *symbol_find(char *s);
int symbol_remove_i(int index);
int symbol_remove_name(char *s);

#endif // SYMBOL_H_
