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
#include "constants.h"

/**
   Here are all the builtin functions. Some of them are also special operators,
   meaning that their arguments are not evaluated.

   Parameters:
   - expr *arg: a list containing the arguments. It is a "proper" lisp list in the
           sense that it is built up of cons cells.
   - expr **out: output argument

   Return value:
      -1 on error, 0 on success.
*/


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
int bi_defun(expr *arg, expr **out) {
    if (list_length(arg) < 2) return NUMBER_OF_ARGUMENTS_ERROR;

    expr *curr = arg->car;
    for_each(curr) {
        if (curr->car->type != SYMBOL) return TYPE_ERROR;

        if (strcmp((char *)curr->car->data, REST_ARGUMENTS_STR) == 0) {
            if (curr->cdr == NULL) {
                printf("ERROR: BUILTIN: DEFUN: expected a symbol after &rest keyword, but got nothing\n");
                return NUMBER_OF_ARGUMENTS_ERROR;
            }
        }
    }

    char *buf = my_malloc(TOKEN_STR_MAX_LEN);
    char *defun_name = (char *)(arg->car->car->data);
    strcpy(buf, defun_name);

    symbol *new_sym = symbol_create(buf, FUNCTION, arg);

    symbol_add(new_sym);
    *out = arg;
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
int bi_define(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    if (arg->car->type != SYMBOL) return TYPE_ERROR;

    /* The second argument needs to be evaluated */
    expr *evaluated_arg;
    if ((ret_code = eval(arg->cdr->car, &evaluated_arg) < 0)) return ret_code;

    char *buf = my_malloc(TOKEN_STR_MAX_LEN);
    /* The name of the symbol exist directly as a string in the first argument. */
    strcpy(buf, (char *)arg->car->data);
    symbol *new_sym = symbol_create(buf, VARIABLE, evaluated_arg);

    symbol_add(new_sym);
    *out = evaluated_arg;
    return 0;
}

int bi_add(expr *arg, expr **out) {
    int acc = 0;
    expr *curr = arg;
    for_each(curr) {
        if (curr->car->type != NUMBER) return TYPE_ERROR;
        acc += (int)curr->car->data;
    }
    expr *_out = expr_new(NUMBER, (uint64_t)acc, NULL, NULL);
    *out = _out;
    return 0;
}

int bi_sub(expr *arg, expr **out) {
    expr *curr = arg;
    for_each(curr) {
        if (curr->car->type != NUMBER) return TYPE_ERROR;
    }

    int acc = 0, arg_length = list_length(arg);
    if (arg_length == 0) {
        acc = 0;
    } else if (arg_length == 1) {
        acc = (int)arg->car->data * -1;
    } else {
        acc = (int)arg->car->data;
        expr *curr = arg->cdr;
        for_each(curr) {
            acc -= (int)curr->car->data;
        }
    }
    *out = expr_new(NUMBER, acc, NULL, NULL);
    return 0;
}

int bi_cons(expr *arg, expr **out) {
    unsigned int size = list_length(arg);
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    *out = expr_cons(arg->car, arg->cdr->car);
    return 0;
}


int bi_car(expr *arg, expr **out) {
    if (arg == NULL) {
        *out = NULL;
        return 0;
    }
    if (arg->car->type != CONS) return TYPE_ERROR;
    *out = arg->car->car;
    return 0;
}

int bi_cdr(expr *arg, expr **out) {
    if (arg == NULL) {
        *out = NULL;
        return 0;
    }
    if (arg->car->type != CONS) return TYPE_ERROR;
    *out = arg->car->cdr;
    return 0;
}

int bi_progn(expr *arg, expr **out) {
    if (arg == NULL) {
        *out = NULL;
        return 0;
    }
    int ret_code;
    expr *curr = arg, *_out;
    for_each(curr) {
        if ((ret_code = eval(curr->car, &_out)) < 0) return ret_code;
    }
    *out = _out;
    return 0;
}

int bi_if(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 3) return NUMBER_OF_ARGUMENTS_ERROR;

    expr *_eval;
    if ((ret_code = eval(arg->car, &_eval)) < 0) return ret_code;
    if (expr_is_true(_eval)) {
        ret_code = eval(arg->cdr->car, out);
    } else {
        ret_code = eval(arg->cdr->cdr->car, out);
    }
    if (ret_code < 0) return ret_code;
    return 0;
}

int bi_print(expr *arg, expr **out) {
    int ret_code;
    expr *curr = arg;
    for_each(curr) {
        if ((ret_code = expr_print(curr->car)) < 0) return ret_code;
    }
    *out = arg->car;
    return 0;
}

int bi_equal(expr *arg, expr **out) {
    int val, arg_length = list_length(arg);
    if (arg_length == 0) {
        val = 0;
    } else if (arg_length == 1) {
        val = 1;
    } else {
        val = 1;
        expr *prev = NULL, *curr = arg;
        for_each(curr) {
            if (!prev) {
                prev = curr;
                continue;
            }
            if ((val = expr_is_equal(prev->car, curr->car)) < 0) return val;
            if (val == 0)
                break;
            prev = curr;
        }
    }
    *out = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    return 0;
}

int bi_gt(expr *arg, expr **out) {
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    int val = expr_gt_lt(arg->car, arg->cdr->car, 1);
    *out = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    return 0;
}

int bi_lt(expr *arg, expr **out) {
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    int val = expr_gt_lt(arg->car, arg->cdr->car, 0);
    *out = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    return 0;
}

int bi_and(expr *arg, expr **out) {
    int val, ret_code, arg_length = list_length(arg);
    if (arg_length == 0) {
        val = 0;
    } else {
        val = 1;
        expr *curr = arg;
        for_each(curr) {
            expr *_eval;
            if ((ret_code = eval(curr->car, &_eval)) < 0) return ret_code;
            if ((val = expr_is_true(_eval)) < 0) return val;
            if (val == 0) break;
        }
    }
    *out = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    return 0;
}

int bi_or(expr *arg, expr **out) {
    int val, ret_code, arg_length = list_length(arg);
    if (arg_length == 0) {
        val = 0;
    } else {
        val = 0;
        expr *curr = arg;
        for_each(curr) {
            expr *_eval;
            if ((ret_code = eval(curr->car, &_eval)) < 0) return ret_code;
            if ((val = expr_is_true(_eval)) < 0) return val;
            if (val == 1) break;
        }
    }

    expr* new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    *out = new_expr;
    return 0;
}

int bi_quote(expr *arg, expr **out) {
    if (list_length(arg) != 1) return NUMBER_OF_ARGUMENTS_ERROR;
    *out = arg->car;
    return 0;
}

int bi_quasiquote(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 1) return NUMBER_OF_ARGUMENTS_ERROR;
    expr *copy;
    if ((ret_code = expr_copy(arg->car, &copy)) < 0) return ret_code;
    if ((ret_code = quasiquote_eval(&copy)) < 0) return ret_code;
    *out = copy;
    return 0;
}

/** If this builtin is triggered as part of normal evaluation, something is wrong.
   All comma signs should be within a quasiquote, and will be handled by quasitquote
   without evaluating the comma symbol itself */
int bi_comma(expr *arg, expr **out) {
    printf("ERROR: BUILTIN: COMMA: encountered comma outside quasiquote\n");
    return -1;
}
/** If this builtin is triggered as part of normal evaluation, something is wrong.
   All comma-at signs should be within a quasiquote, and will be handled by quasitquote
   without evaluating the comma-at symbol itself */
int bi_comma_at(expr *arg, expr **out) {
    printf("ERROR: BUILTIN: COMMA-at: encountered comma-at outside quasiquote\n");
    return -1;
}

int bi_defmacro(expr *arg, expr **out) {
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    expr *curr = arg->car;
    for_each(curr) {
        if (curr->car->type != SYMBOL) return TYPE_ERROR;
    }
    char *buf = my_malloc(TOKEN_STR_MAX_LEN);
    strcpy(buf, (char *)arg->car->car->data);
    symbol_add(symbol_create(buf, MACRO, arg));
    *out = arg;
    return 0;
}

int bi_macroexpand(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 1) return NUMBER_OF_ARGUMENTS_ERROR;
    symbol *sym = symbol_find((char *) arg->car->car->data);
    if (sym == NULL) return UNBOUND_SYMBOL_NAME_ERROR;
    expr *macro_expand;
    if ((ret_code = function_invocation(sym, arg->car->cdr, &macro_expand)) < 0)
        return ret_code;
    *out = macro_expand;
    return 0;
}
