#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ir.h"

// #include "eval.h"

int add(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to add \n");
        return -1;
    }
    uint64_t acc = 0;
    while (arg) {
        if (arg->type != NUMBER) {
            printf("TYPE ERROR: Add can only handle numbers\n");
            return -1;
        }
        acc += arg->data;
        arg = arg->next;
    }

    expr *_res = malloc(sizeof(expr));
    _res->type = NUMBER;
    _res->data = acc;
    _res->next = NULL;
    *res = _res;
    return 0;
}

int sub(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to sub \n");
        return -1;
    }
    if (arg->next == NULL) {
        if (arg->type != NUMBER) {
            printf("TYPE ERROR: Sub can only handle numbers\n");
            return -1;
        }
        return arg->data * (-1);
    }

    uint64_t acc = arg->data;
    arg = arg->next;
    while (arg) {
        if (arg->type != NUMBER) {
            printf("TYPE ERROR: Sub can only handle numbers\n");
            return -1;
        }
        acc -= arg->data;
        arg = arg->next;
    }
    expr *_res = malloc(sizeof(expr));
    _res->type = NUMBER;
    _res->data = acc;
    _res->next = NULL;
    *res = _res;
    return 0;
}

int cons(expr *arg, expr **res) {
    if (arg == NULL || arg->next == NULL || arg->next->next != NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Cons accepts exactly two arguments \n");
        return -1;
    }

    expr *cons = malloc(sizeof(expr));
    cons->type = CONS;

    expr *exprs = (expr *)malloc(2 * sizeof(expr));
    exprs[0].type = arg->type;
    exprs[0].data = arg->data;
    exprs[0].next = NULL;
    exprs[1].type = arg->next->type;
    exprs[1].data = arg->next->data;
    exprs[1].next = NULL;

    cons->data = (uint64_t) exprs;
    *res = cons;
    return 0;
}


int car(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to car \n");
        return -1;
    }
    if (arg->type != CONS) {
        printf("ERROR: Car can only handle cons cells\n");
        return -1;
    }
    *res = &((expr *)arg->data)[0];
    return 0;
}

int cdr(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to cdr \n");
        return -1;
    }
    if (arg->type != CONS) {
        printf("ERROR: Cdr can only handle cons cells\n");
        return -1;
    }

    *res = &((expr *)arg->data)[1];
    return 0;
}

expr *eval(expr *e) {
    switch (e->type) {
        case NUMBER:
            return e;
        case SYMBOL:
            return e;
        case PROC_START:
            return eval((expr *)e->data);
        case PROC_SYMBOL:;
            expr *proc = e;
            expr *arg = e->next;

            /* Evaluate the arguments, and build a list of those evaluated arguments */
            expr *prev_arg = NULL;
            expr *first_arg = NULL;
            while (arg) {
                expr *ev = eval(arg);
                // uint64_t val = eval(arg);

                expr *new = malloc(sizeof(expr));
                new->type = ev->type;
                new->data = ev->data;
                new->next = NULL;
                if (!first_arg)
                    first_arg = new;
                if (prev_arg)
                    prev_arg->next = new;
                prev_arg = new;
                arg = arg->next;
            }

            expr *res;
            if (strcmp((char *)(proc->data), "+") == 0) {
                add(first_arg, &res);
            }
            else if (strcmp((char *)(proc->data), "-") == 0) {
                sub(first_arg, &res);
            }
            else if (strcmp((char *)(proc->data), "cons") == 0) {
                cons(first_arg, &res);
            }
            else if (strcmp((char *)(proc->data), "car") == 0) {
                car(first_arg, &res);
            }
            else if (strcmp((char *)(proc->data), "cdr") == 0) {
                cdr(first_arg, &res);
            }
            else {
                printf("ERROR: No function identified for proc %s\n", (char*)(proc->data));
            }
            return res;
        default:
            printf("no action\n");
            return 0;
    }
}
