#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "memory.h"
#include "list.h"


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
expr *expr_cons(expr* car, expr *cdr) {
    return expr_new(CONS, 0, car, cdr);
}

int expr_copy(expr* e, expr **out) {
    if (e == NULL) {
        *out = NULL;
        return 0;
    }
    switch (e->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            *out = expr_new(e->type, e->data, NULL, NULL);
            return 0;
        case SYMBOL:;
            char *data = malloc(strlen((char *)e->data));
            if (data == NULL) {
                printf("ERROR: EXPR: COPY: malloc error \n");
                return -1;
            }
            strcpy(data, (char *)e->data);
            *out = expr_new(e->type, (uint64_t)data, NULL, NULL);
            return 0;
        case CONS:;
            expr *car, *cdr;
            int cpy_res = expr_copy(e->car, &car);
            if (cpy_res < 0) {
                printf("ERROR: EXPR: COPY: error for car field of cons cell\n");
                return cpy_res;
            }
            cpy_res = expr_copy(e->cdr, &cdr);
            if (cpy_res < 0) {
                printf("ERROR: EXPR: COPY: error for cdr field of cons cell\n");
                return cpy_res;
            }
            *out = expr_cons(car, cdr);
            return 0;
        default:
            printf("ERROR: EXPR: COPY: unknown type\n");
            return -1;
    }
}

int expr_is_true(expr *e) {
    if (e == NULL)
        return 0;

    switch (e->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            return (int)e->data;
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

    switch (e1->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            if (!(e2->type == NUMBER || e2->type == CHAR || e2->type == BOOLEAN)) {
                printf("ERROR: EXPR: EXPR_IS_EQUAL: equal can only compare number, char or boolean with number, char or boolean\n");
                return -1;
            }
            return (int)e1->data == (int)e2->data;
        case SYMBOL:
            if (e2->type != SYMBOL) {
                printf("ERROR: EXPR: EXPR_IS_EQUAL: trying to compare symbol with something else\n");
                return -1;
            }
            /* TODO: This is stupid, find another way. Even if the name is the same the scope might be different */
            int cmp_eq = strcmp((char *)e1->data, (char *)e2->data);
            return cmp_eq ? 0 : 1;
        case CONS:;
            if (e2->type != CONS) {
                printf("ERROR: EXPR: EXPR_IS_EQUAL: trying to compare cons cell with something else\n");
                return -1;
            }

            if (list_length(e1) != list_length(e2))
                return 0;
            expr *curr1 = e1, *curr2 = e2;
            if (expr_is_equal(curr1->car, curr2->car) == 0)
                return 0;
            return expr_is_equal(curr1->cdr, curr2->cdr);

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

    if (e2->type == SYMBOL) {
        printf("ERROR: EXPR: GT/LT: cant use gt/lt on symbol\n");
        return -1;
    }
    if (e2->type == CONS) {
        printf("ERROR: EXPR: GT/LT: cant use gt/lt on cons\n");
        return -1;
    }

    switch (e1->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            if (!(e2->type == NUMBER || e2->type == CHAR || e2->type == BOOLEAN)) {
                printf("ERROR: EXPR: GT/LT: can only compare number, char or boolean with number, char or boolean\n");
                return -1;
            }
            if (gt_or_lt)
                return (int)e1->data > (int)e2->data;
            return (int)e1->data < (int)e2->data;
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
