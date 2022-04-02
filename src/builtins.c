#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "parser.h"
#include "eval.h"
#include "list.h"
#include "expr.h"
#include "symbol.h"
#include "memory.h"
#include "config.h"

/**
   Example defun: (defun (add x y) (+ x y)), here (add x y) is
   the first argument to the defun and (+ x y) is the second argument
   to the defun. The first argument (add x y) contains the name of the
   function and then the arguments. The first entry in the first argument
   is the name of the function, the rest of the entries are the function
   arguments.

   A defun is an expr, where the ->data field points to the
   first argument (which is again is a list of arguments, where
   ->data field points to the actual list of those arguments). Then,
   the ->next field of the first argument points to the the second
   field which is the logic of the function (also a list).

   Illustraion, vertical lines are ->cdr field, horizontal lines are ->car field
   (of course, still need to use car after cdr to get the actual element data)


   args        cdr-->         first list of function logic    cdr-->     next list of function logic
     |                                     |                                   |
    car                                   car                                 car
     |                                     |                                   |
   f_name --> arg1 --> arg2         list of function logic               list of function logic

*/
int bi_defun(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("EVAL: ERROR: Got NULL argument for defun \n");
        return -1;
    }

    expr *curr_arg = arg->car;
    while (!list_end(curr_arg)) {
        if (curr_arg->car->type != SYMBOL) {
            printf("EVAL: ERROR: Expected all arguments to defun to be symbols, but found one that was of type %s\n",
                   type_str(curr_arg->car->type));
            return -1;
        }
        curr_arg = curr_arg->cdr;
    }

    char *buf = my_malloc(MAX_TOKEN_LENGTH);
    char *defun_name = (char *)(arg->car->car->data);
    strcpy(buf, defun_name);

    symbol *new_sym = symbol_create(buf, FUNCTION, arg);

    symbol_add(new_sym);
    *res = arg;
    return 0;
}


/**
   Defines a variable with a value.

   Example define: (define x (+ 1 2)), which sets the variable
   x to the value 3.

   The name of the variable will be in the first arguments ->data field,
   directly as a string. The value of the variable will be in the second argument.

   Define is special in the sense that it accepts an undefined symbol as its first
   argument, so we should not evaluate this argument. The second argument however, needs
   to be evaluated.
*/
int bi_define(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("EVAL: ERROR: Nothing to define\n");
        return -1;
    }
    unsigned int arg_count = list_length(arg);
    if (arg_count != 2) {
        printf("EVAL: ERROR: Define needs exactly two arguments, but got %d\n", arg_count);
        return -1;
    }

    if (arg->car->type != SYMBOL) {
        printf("EVAL: ERROR: The first argument to define must be a symbol, but got %s\n",
               type_str(arg->car->type));
        return -1;
    }

    /* The second argument needs to be evaluated */
    expr *evaluated_arg = eval(arg->cdr->car);

    char *buf = my_malloc(MAX_TOKEN_LENGTH);
    /* The name of the symbol exist directly as a string in the first argument. */
    strcpy(buf, (char *)arg->car->data);
    symbol *new_sym = symbol_create(buf, VARIABLE, evaluated_arg);

    symbol_add(new_sym);
    *res = evaluated_arg;
    return 0;
}

int bi_add(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("BUILTIN: ERROR: Nothing to add \n");
        return -1;
    }
    int acc = 0;
    while (!list_end(arg)) {
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
    while (!list_end(arg)) {
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
    /* Car is the first argument to the car builtin, and since
       the first argument is a cons cell, we need the car of that
       as well */
    *res = arg->car->car;
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
    /* Car is the first argument to the car builtin, and since
       the first argument is a cons cell, we need the cdr of that
       as well */
    *res = arg->car->cdr;
    return 0;
}

int bi_progn(expr *arg, expr **res) {
    if (arg == NULL) {
        printf("BUILTIN: ERROR: Nothing to progn\n");
        return -1;
    }
    expr *curr_arg = arg, *_res;
    while (!list_end(curr_arg)) {
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
    unsigned int arg_count = list_length(arg);
    if (arg_count != 3) {
        printf("BUILTIN: ERROR: \"if\" needs exactly three arguments, but got %d \n", arg_count);
        return -1;
    }

    if (expr_is_true(eval(arg->car))) {
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
