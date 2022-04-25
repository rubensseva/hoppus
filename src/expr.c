#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "memory.h"
#include "list.h"
#include "config.h"

#define CDR_UNTAG(cdr) ((expr *)(((uint64_t) cdr) & 0xFFFFFFFFFFFFFFFE))
#define CDR_IS_CONS(cdr) (!((expr *)(((uint64_t) (cdr)) & 1)))
#define CDR_OTHER_TYPE(cdr) ((expr_type)(((uint64_t) (cdr)) >> 1))

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


expr *car(expr *e) {
    return e->car;
};
void set_car(expr *e, expr *new_car) {
    e->car = new_car;
};
/* TODO: If we want the cdr of an expression, then we should know that the expr is
   cons cell, in which case the LSB should be 0, so there is no need to mask
   the value. But could be nice to do this in any case, to avoid future bugs? */
expr *cdr(expr *e) {
    return (expr *)((uint64_t) e->cdr & 0xFFFFFFFFFFFFFFFE);
};
void set_cdr(expr *e, expr* new_cdr) {
    e->cdr = (expr *)((uint64_t) CDR_UNTAG(new_cdr) | !CDR_IS_CONS(e->cdr));
}
uint64_t data(expr *e) {
    return (uint64_t) car(e);
}
void set_data(expr *e, uint64_t data) {
    e->car = (expr *)data;
}
expr_type type(expr *e) {
    if (CDR_IS_CONS(e->cdr))
        return CONS;
    return CDR_OTHER_TYPE(e->cdr);
};
void set_type(expr *e, expr_type type) {
    if (type == CONS) {
        e->cdr = (expr *)((uint64_t)(e->cdr) & 0xFFFFFFFFFFFFFFFE);
    } else {
        e->cdr = (expr *)((((uint64_t) type) << 1) | 1);
    }
};

expr_type tar(expr *e) {
    return type(car(e));
}
uint64_t dar(expr *e) {
    return data(car(e));
}

expr *nth(unsigned int i, expr *e) {
    unsigned int count = 0;
    for (expr *curr = e; !list_end(e); e = cdr(e)) {
        if (count == i)
            return car(curr);
    }
    return NULL;
}

expr *expr_new() {
    expr *new = (expr *) my_malloc(sizeof(expr));
    new->car = NULL;
    new->cdr = NULL;
    return new;
}

expr *expr_new_val(expr_type type, uint64_t data) {
    expr *new = expr_new();
    set_type(new, type);
    set_data(new, data);
    return new;
};

expr *expr_new_cons(expr* car, expr *cdr) {
    expr *new = expr_new();
    set_type(new, CONS);
    set_car(new, car);
    set_cdr(new, cdr);
    return new;
}

int expr_copy(expr* e, expr **out) {
    if (e == NULL) {
        *out = NULL;
        return 0;
    }
    switch (type(e)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            *out = expr_new_val(type(e), data(e));
            return 0;
        case SYMBOL:;
            /* We could copy the string as well here, but a symbol name
               shouldnt really ever change, so should be fine to just use
               the same string */
            *out = expr_new_val(type(e), data(e));
            return 0;
        case CONS:;
            expr *_car, *_cdr;
            int cpy_res = expr_copy(car(e), &_car);
            if (cpy_res < 0) {
                printf("ERROR: EXPR: COPY: error for car field of cons cell\n");
                return cpy_res;
            }
            cpy_res = expr_copy(cdr(e), &_cdr);
            if (cpy_res < 0) {
                printf("ERROR: EXPR: COPY: error for cdr field of cons cell\n");
                return cpy_res;
            }
            *out = expr_new_cons(_car, _cdr);
            return 0;
        default:
            printf("ERROR: EXPR: COPY: unknown type\n");
            return -1;
    }
}

int expr_is_true(expr *e) {
    if (e == NULL)
        return 0;

    switch (type(e)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            return (int)data(e);
        case SYMBOL:
            return 1;
        case CONS:
            return 1;
        default:
            printf("ERROR: EXPR: EXPR_IS_TRUE: Got unknown type\n");
            return 0;
    }
}

int expr_is_equal(expr *e1, expr *e2) {
    if (e1 == NULL || e2 == NULL)
        return e1 == e2;

    switch (type(e1)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            if (!(type(e2) == NUMBER || type(e2) == CHAR || type(e2) == BOOLEAN)) {
                printf("ERROR: EXPR: EXPR_IS_EQUAL: equal can only compare number, char or boolean with number, char or boolean\n");
                return -1;
            }
            return (int)data(e1) == (int)data(e2);
        case SYMBOL:
            if (type(e2) != SYMBOL) {
                printf("ERROR: EXPR: EXPR_IS_EQUAL: trying to compare symbol with something else\n");
                return -1;
            }
            /* TODO: Perhaps just comparing the pointers is better here? */
            int cmp_eq = strcmp((char *)data(e1), (char *)data(e2));
            return cmp_eq ? 0 : 1;
        case CONS:;
            if (type(e2) != CONS) {
                printf("ERROR: EXPR: EXPR_IS_EQUAL: trying to compare cons cell with something else\n");
                return -1;
            }

            if (list_length(e1) != list_length(e2))
                return 0;
            expr *curr1 = e1, *curr2 = e2;
            if (expr_is_equal(car(curr1), car(curr2)) == 0)
                return 0;
            return expr_is_equal(cdr(curr1), cdr(curr2));

        default:
            printf("ERROR: EXPR: EXPR_IS_EQUAL: got unknown type when checking if true or false\n");
            return -1;
    }
}

/**
  If gt_or_lt is true, calculate gt, otherwise lt
*/
int expr_gt_lt(expr *e1, expr *e2, int gt_or_lt) {
    if (e1 == NULL || e2 == NULL)
        return e1 == e2;

    if (type(e2) == SYMBOL) {
        printf("ERROR: EXPR: GT/LT: cant use gt/lt on symbol\n");
        return -1;
    }
    if (type(e2) == CONS) {
        printf("ERROR: EXPR: GT/LT: cant use gt/lt on cons\n");
        return -1;
    }

    switch (type(e1)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            if (!(type(e2) == NUMBER || type(e2) == CHAR || type(e2) == BOOLEAN)) {
                printf("ERROR: EXPR: GT/LT: can only compare number, char or boolean with number, char or boolean\n");
                return -1;
            }
            if (gt_or_lt)
                return (int)data(e1) > (int)data(e2);
            return (int)data(e1) < (int)data(e2);
        case SYMBOL:
            printf("ERROR: EXPR: GT/LT: cant use gt/lt on symbol\n");
            return -1;
        case CONS:;
            printf("ERROR: EXPR: GT/LT: cant use gt/lt on cons\n");
            return -1;
        default:
            printf("ERROR: EXPR: GT/LT: got unknown type when checking if true or false\n");
            return -1;
    }
}


int expr_is_str(expr *e) {
    expr *curr = e;
    for_each(curr) {
        if (type(car(curr)) != CHAR) {
            return 0;
        }
    }
    return 1;
}

int str_from_expr(expr *e, char **out) {
    unsigned int length = list_length(e);
    if (length >= LISP_STR_MAX_LEN) {
        printf("ERROR: EXPR: String is too large: %d\n", length);
        return -1;
    }
    char *str = (char *) my_malloc(length + 1);
    expr *curr = e;
    int i = 0;
    for_each(curr) {
        str[i++] = data(car(curr));
    }
    str[i] = '\0';
    *out = str;
    return 0;
}

int expr_print_tree(expr *e) {
    if (e == NULL) {
        printf("nil");
        return 0;
    }
    switch(type(e)) {
        case NUMBER:
            printf("%d", (int)data(e));
            return 0;
        case CHAR:
            printf("'%c'", (char)data(e));
            return 0;
        case SYMBOL:
            printf("%s", (char *)data(e));
            return 0;
        case CONS:;
            if (expr_is_str(e)) {
                char *str;
                int ret_code = str_from_expr(e, &str);
                if (ret_code < 0) {
                    printf("ERROR: EXPR: PRINT: Converting expr to string\n");
                    return -1;
                }
                printf("\"");
                printf("%s", str);
                printf("\"");
                return 0;
            }
            printf("(");
            expr *curr = e;
            for_each(curr) {
                expr_print_tree(car(curr));
                if (!list_end(cdr(curr))) {
                    printf(" ");
                }
            }
            printf(")");
            return 0;
        case BOOLEAN:
            if (data(e)) {
                printf("true");
            } else {
                printf("false");
            }
            return 0;
        default:
            printf("ERROR: EXPR: PRINT: Got unknown expr type\n");
            return -1;
    }
}

int expr_print(expr *e) {
    expr_print_tree(e);
    printf("\n");
    return 0;
}
