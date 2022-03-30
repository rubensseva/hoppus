#include <stdint.h>
#include <stdio.h>

#include "builtins.h"
#include "ir.h"


int bi_add(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to add \n");
        return -1;
    }
    uint64_t acc = 0;
    while (arg) {
        if (arg->car->type != NUMBER) {
            printf("TYPE ERROR: Add can only handle numbers\n");
            return -1;
        }
        acc += arg->car->data;
        arg = arg->cdr;
    }

    expr *_res = expr_new(NUMBER, acc, NULL, NULL);
    *res = _res;
    return 0;
}

int bi_sub(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to sub \n");
        return -1;
    }
    if (arg->cdr == NULL) {
        if (arg->car->type != NUMBER) {
            printf("TYPE ERROR: Sub can only handle numbers\n");
            return -1;
        }
        return arg->data * (-1);
    }

    uint64_t acc = arg->car->data;
    arg = arg->cdr;
    while (arg) {
        if (arg->car->type != NUMBER) {
            printf("TYPE ERROR: Sub can only handle numbers\n");
            return -1;
        }
        acc -= arg->car->data;
        arg = arg->cdr;
    }

    expr *_res = expr_new(NUMBER, acc, NULL, NULL);
    *res = _res;
    return 0;
}

int bi_cons(expr *arg, expr **res) {
    if (arg == NULL || arg->cdr == NULL || arg->cdr->cdr != NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Cons accepts exactly two arguments \n");
        return -1;
    }

    *res = expr_cons(arg->car, arg->cdr->car);
    return 0;
}


int bi_car(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to car \n");
        return -1;
    }
    if (arg->car->type != CONS) {
        printf("ERROR: Car can only handle cons cells\n");
        return -1;
    }
    *res = arg->car;
    return 0;
}

int bi_cdr(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to cdr \n");
        return -1;
    }
    if (arg->car->type != CONS) {
        printf("ERROR: Cdr can only handle cons cells\n");
        return -1;
    }
    *res = arg->cdr;
    return 0;
}
