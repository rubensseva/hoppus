#ifndef SYMBOL_H_
#define SYMBOL_H_

#include "ir.h"

typedef int builtin_fn_t(expr *arg, expr **res);

typedef enum symbol_type_t {
    FUNCTION,
    BUILTIN,
    VARIABLE
} symbol_type;

typedef struct symbol_t {
    char *name;
    symbol_type type;
    expr *e;
    builtin_fn_t *builtin_fn;
} symbol;

symbol *symbol_create(char *name, symbol_type type, expr *e);
symbol *symbol_builtin_create(char *name, builtin_fn_t *fn);
void symbol_add(symbol *s);
symbol *symbol_find(char *s);
int symbol_remove_i(int index);
int symbol_remove_name(char *s);

#endif // SYMBOL_H_
