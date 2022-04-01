#include <stdint.h>
#include <stdio.h>

#include "builtins.h"
#include "parser.h"
#include "eval.h"


int bi_add(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("BUILTIN: ERROR: Nothing to add \n");
        return -1;
    }
    int acc = 0;
    while (arg) {
        if (arg->car == NULL) {
            printf("BUILTIN: ERROR: Argument to add had a car that was NULL\n");
            return -1;
        }
        if (arg->car->type != NUMBER) {
            printf("BUILTIN: ERROR: Add can only handle numbers but got %s\n",
                   type_str(arg->car->type));
            return -1;
        }
        acc += (int)arg->car->data;
        arg = arg->cdr;
    }

    expr *_res = expr_new(NUMBER, (uint64_t)acc, NULL, NULL);
    *res = _res;
    return 0;
}

int bi_sub(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("BUILTIN: ERROR: Nothing to sub \n");
        return -1;
    }
    /* If only one argument, return the negative of that argument */
    if (arg->cdr == NULL) {
        if (arg->car->type != NUMBER) {
            printf("BUILTIN: ERROR: Sub can only handle numbers but got %s\n",
                   type_str(arg->car->type));
            return -1;
        }
        int val = (int)arg->car->data * -1;
        expr *_res = expr_new(NUMBER, val, NULL, NULL);
        *res = _res;
        return 0;
    }

    int acc = (int)arg->car->data;
    arg = arg->cdr;
    while (arg) {
        if (arg->car == NULL) {
            printf("BUILTIN: ERROR: Argument to sub had a car that was NULL\n");
            return -1;
        }
        if (arg->car->type != NUMBER) {
            printf("BUILTIN: ERROR: Sub can only handle numbers but got %s\n",
                   type_str(arg->car->type));
            return -1;
        }
        acc -= (int)arg->car->data;
        arg = arg->cdr;
    }

    expr *_res = expr_new(NUMBER, acc, NULL, NULL);
    *res = _res;
    return 0;
}

int bi_cons(expr *arg, expr **res) {
    if (arg == NULL || arg->cdr == NULL || arg->cdr->cdr != NULL) {
        printf("BUILTIN: ERROR: Cons accepts exactly two arguments \n");
        return -1;
    }

    *res = expr_cons(arg->car, arg->cdr->car);
    return 0;
}


int bi_car(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("BUILTIN: ERROR: Nothing to car \n");
        return -1;
    }
    if (arg->car->type != CONS) {
        printf("BUILTIN: ERROR: Car can only handle numbers but got %s\n",
               type_str(arg->car->type));
        return -1;
    }
    *res = arg->car;
    return 0;
}

int bi_cdr(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("BUILTIN: ERROR: Nothing to cdr \n");
        return -1;
    }
    if (arg->car->type != CONS) {
        printf("BUILTIN: ERROR: Cdr can only handle numbers but got %s\n",
               type_str(arg->car->type));
        return -1;
    }
    *res = arg->cdr;
    return 0;
}

int bi_progn(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("BUILTIN: ERROR: Nothing to progn\n");
        return -1;
    }
    expr *curr_arg = arg, *_res;
    while (curr_arg) {
        _res = eval(curr_arg->car);
        curr_arg = curr_arg->cdr;
    }
    *res = _res;
    return 0;
}

int bi_if(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("BUILTIN: ERROR: Nothing to progn\n");
        return -1;
    }
    if (arg->car == NULL ||
        arg->cdr == NULL || arg->cdr->car == NULL ||
        arg->cdr->cdr == NULL || arg->cdr->cdr->car == NULL ||
        arg->cdr->cdr->cdr != NULL) {
        printf("BUILTIN: ERROR: \"if\" needs exactly three arguments\n");
        return -1;
    }
    if (arg->car->type != NUMBER) {
        printf("BUILTIN: ERROR: \"if\" currently only supports numbers\n");
        return -1;
    }

    if (arg->car->data) {
        *res = eval(arg->cdr->car);
    } else {
        *res = eval(arg->cdr->cdr->car);
    }

    return 0;
}

int bi_print(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("BUILTIN: ERROR: Nothing to print\n");
        return -1;
    }
    int print_res = print_expr(arg->car);
    *res = arg->car;
    return print_res;
}
