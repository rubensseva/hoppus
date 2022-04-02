#include <stdio.h>

#include "expr.h"
#include "memory.h"

char *unknown_type_str = "unknown";
char *expr_type_string_map[EXPR_TYPE_ENUM_SIZE] = {
    "symbol",
    "number",
    "char",
    "cons",
    "boolean"
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

/**
  Check if an expression is true or false
*/
int expr_is_true(expr *e) {
    switch (e->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            return (int)e->data;
        case SYMBOL:
            return 1;
        case CONS:
            return (e->car && e->cdr);
        default:
            printf("EXPR: ERROR: Got unknown type when checking if true or false\n");
            return 0;
    }
}
