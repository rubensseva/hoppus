#include <string1.h>
#include <link.h>

#include <expr.h>
#include <list.h>
#include <clisp_memory.h>
#include <clisp_config.h>

#define CDR_UNTAG(cdr) ((expr *)(((uint32_t) cdr) & 0xFFFFFFFE))
#define CDR_IS_CONS(cdr) (!((expr *)(((uint32_t) (cdr)) & 1)))
#define CDR_OTHER_TYPE(cdr) ((expr_type)(((uint32_t) (cdr)) >> 1))

__USER_DATA char SYMBOL_TYPE_STR[] = "symbol";
__USER_DATA char NUMBER_TYPE_STR[] = "number";
__USER_DATA char CHAR_TYPE_STR[] = "char";
__USER_DATA char CONS_TYPE_STR[] = "cons";
__USER_DATA char BOOLEAN_TYPE_STR[] = "boolean";

__USER_DATA char unknown_type_str[] = "unknown";
__USER_DATA char *expr_type_string_map[EXPR_TYPE_ENUM_SIZE] = {
    SYMBOL_TYPE_STR,
    NUMBER_TYPE_STR,
    CHAR_TYPE_STR,
    CONS_TYPE_STR,
    BOOLEAN_TYPE_STR
};

__USER_TEXT char *type_str(expr_type tp) {
    if (tp < 0 || tp > EXPR_TYPE_ENUM_SIZE) {
        return unknown_type_str;
    }
    return expr_type_string_map[tp];
}


__USER_TEXT expr *car(expr *e) {
    return e->car;
};
__USER_TEXT void set_car(expr *e, expr *new_car) {
    e->car = new_car;
};
/* TODO: If we want the cdr of an expression, then we should know that the expr is
   cons cell, in which case the LSB should be 0, so there is no need to mask
   the value. But could be nice to do this in any case, to avoid future bugs? */
__USER_TEXT expr *cdr(expr *e) {
    return (expr *)((uint32_t) e->cdr & 0xFFFFFFFE);
};
__USER_TEXT void set_cdr(expr *e, expr* new_cdr) {
    e->cdr = (expr *)((uint32_t) CDR_UNTAG(new_cdr) | !CDR_IS_CONS(e->cdr));
}
__USER_TEXT uint32_t data(expr *e) {
    return (uint32_t) car(e);
}
__USER_TEXT void set_data(expr *e, uint32_t data) {
    e->car = (expr *)data;
}
__USER_TEXT expr_type type(expr *e) {
    if (CDR_IS_CONS(e->cdr))
        return CONS;
    return CDR_OTHER_TYPE(e->cdr);
};
__USER_TEXT void set_type(expr *e, expr_type type) {
    if (type == CONS) {
        e->cdr = (expr *)((uint32_t)(e->cdr) & 0xFFFFFFFE);
    } else {
        e->cdr = (expr *)((((uint32_t) type) << 1) | 1);
    }
};

__USER_TEXT expr_type tar(expr *e) {
    return type(car(e));
}
__USER_TEXT uint32_t dar(expr *e) {
    return data(car(e));
}

__USER_TEXT expr *nth(unsigned int i, expr *e) {
    unsigned int count = 0;
    for (expr *curr = e; !list_end(e); e = cdr(e)) {
        if (count == i)
            return car(curr);
    }
    return NULL;
}

__USER_TEXT expr *expr_new() {
    expr *new = (expr *) my_malloc(sizeof(expr));
    new->car = NULL;
    new->cdr = NULL;
    return new;
}

__USER_TEXT expr *expr_new_val(expr_type type, uint32_t data) {
    expr *new = expr_new();
    set_type(new, type);
    set_data(new, data);
    return new;
};

__USER_TEXT expr *expr_new_cons(expr* car, expr *cdr) {
    expr *new = expr_new();
    set_type(new, CONS);
    set_car(new, car);
    set_cdr(new, cdr);
    return new;
}

__USER_TEXT int expr_copy(expr* e, expr **out) {
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
                user_puts("ERROR: EXPR: COPY: error for car field of cons cell\n");
                return cpy_res;
            }
            cpy_res = expr_copy(cdr(e), &_cdr);
            if (cpy_res < 0) {
                user_puts("ERROR: EXPR: COPY: error for cdr field of cons cell\n");
                return cpy_res;
            }
            *out = expr_new_cons(_car, _cdr);
            return 0;
        default:
            user_puts("ERROR: EXPR: COPY: unknown type\n");
            return -1;
    }
}

__USER_TEXT int expr_is_true(expr *e) {
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
            user_puts("ERROR: EXPR: EXPR_IS_TRUE: Got unknown type\n");
            return 0;
    }
}

__USER_TEXT int expr_is_equal(expr *e1, expr *e2) {
    if (e1 == NULL || e2 == NULL)
        return e1 == e2;

    switch (type(e1)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            if (!(type(e2) == NUMBER || type(e2) == CHAR || type(e2) == BOOLEAN)) {
                user_puts("ERROR: EXPR: EXPR_IS_EQUAL: equal can only compare number, char or boolean with number, char or boolean\n");
                return -1;
            }
            return (int)data(e1) == (int)data(e2);
        case SYMBOL:
            if (type(e2) != SYMBOL) {
                user_puts("ERROR: EXPR: EXPR_IS_EQUAL: trying to compare symbol with something else\n");
                return -1;
            }
            /* TODO: Perhaps just comparing the pointers is better here? */
            int cmp_eq = strcmp1((char *)data(e1), (char *)data(e2));
            return cmp_eq ? 0 : 1;
        case CONS:;
            if (type(e2) != CONS) {
                user_puts("ERROR: EXPR: EXPR_IS_EQUAL: trying to compare cons cell with something else\n");
                return -1;
            }

            if (list_length(e1) != list_length(e2))
                return 0;
            expr *curr1 = e1, *curr2 = e2;
            if (expr_is_equal(car(curr1), car(curr2)) == 0)
                return 0;
            return expr_is_equal(cdr(curr1), cdr(curr2));

        default:
            user_puts("ERROR: EXPR: EXPR_IS_EQUAL: got unknown type when checking if true or false\n");
            return -1;
    }
}

/**
  If gt_or_lt is true, calculate gt, otherwise lt
*/
__USER_TEXT int expr_gt_lt(expr *e1, expr *e2, int gt_or_lt) {
    if (e1 == NULL || e2 == NULL)
        return e1 == e2;

    if (type(e2) == SYMBOL) {
        user_puts("ERROR: EXPR: GT/LT: cant use gt/lt on symbol\n");
        return -1;
    }
    if (type(e2) == CONS) {
        user_puts("ERROR: EXPR: GT/LT: cant use gt/lt on cons\n");
        return -1;
    }

    switch (type(e1)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            if (!(type(e2) == NUMBER || type(e2) == CHAR || type(e2) == BOOLEAN)) {
                user_puts("ERROR: EXPR: GT/LT: can only compare number, char or boolean with number, char or boolean\n");
                return -1;
            }
            if (gt_or_lt)
                return (int)data(e1) > (int)data(e2);
            return (int)data(e1) < (int)data(e2);
        case SYMBOL:
            user_puts("ERROR: EXPR: GT/LT: cant use gt/lt on symbol\n");
            return -1;
        case CONS:;
            user_puts("ERROR: EXPR: GT/LT: cant use gt/lt on cons\n");
            return -1;
        default:
            user_puts("ERROR: EXPR: GT/LT: got unknown type when checking if true or false\n");
            return -1;
    }
}


__USER_TEXT int expr_is_str(expr *e) {
    expr *curr = e;
    for_each(curr) {
        if (type(car(curr)) != CHAR) {
            return 0;
        }
    }
    return 1;
}

__USER_TEXT int str_from_expr(expr *e, char **out) {
    unsigned int length = list_length(e);
    if (length >= LISP_STR_MAX_LEN) {
        user_printf("ERROR: EXPR: String is too large: %d\n", length);
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

__USER_TEXT int expr_print_tree(expr *e) {
    if (e == NULL) {
        user_puts("nil");
        return 0;
    }
    switch(type(e)) {
        case NUMBER:
            user_printf("%d", (int)data(e));
            return 0;
        case CHAR:
            user_printf("'%c'", (char)data(e));
            return 0;
        case SYMBOL:
            user_printf("%s", (char *)data(e));
            return 0;
        case CONS:;
            if (expr_is_str(e)) {
                char *str;
                int ret_code = str_from_expr(e, &str);
                if (ret_code < 0) {
                    user_puts("ERROR: EXPR: PRINT: Converting expr to string\n");
                    return -1;
                }
                user_puts("\"");
                user_printf("%s", str);
                user_puts("\"");
                return 0;
            }
            user_puts("(");
            expr *curr = e;
            for_each(curr) {
                expr_print_tree(car(curr));
                if (!list_end(cdr(curr))) {
                    user_puts(" ");
                }
            }
            user_puts(")");
            return 0;
        case BOOLEAN:
            if (data(e)) {
                user_puts("true");
            } else {
                user_puts("false");
            }
            return 0;
        default:
            user_puts("ERROR: EXPR: PRINT: Got unknown expr type\n");
            return -1;
    }
}

__USER_TEXT int expr_print(expr *e) {
    expr_print_tree(e);
    user_puts("\n");
    return 0;
}
