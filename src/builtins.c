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
    unsigned int arg_count = list_length(arg);
    if (arg_count < 2) {
        printf("ERROR: BUILTIN: DEFUN: need exactly two arguments, but got %d\n", arg_count);
        return -1;
    }

    expr *curr_arg = arg->car;
    for_each(curr_arg) {
        if (curr_arg->car->type != SYMBOL) {
            printf("ERROR: BUILTIN: DEFUN: expect all arguments to be symbols, but found one that was of type %s\n",
                   type_str(curr_arg->car->type));
            return -1;
        }

        if (strcmp((char *)curr_arg->car->data, REST_ARGUMENTS_STR) == 0) {
            if (curr_arg->cdr == NULL) {
                printf("ERROR: BUILTIN: DEFUN: expect a symbol after &rest keyword, but got nothing\n");
                return -1;
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
    unsigned int arg_count = list_length(arg);
    if (arg_count != 2) {
        printf("ERROR: BUILTIN: DEFINE: need exactly two arguments, but got %d\n", arg_count);
        return -1;
    }

    if (arg->car->type != SYMBOL) {
        printf("ERROR: BUILTIN: DEFINE: the first argument to define must be a symbol, but got %s\n",
               type_str(arg->car->type));
        return -1;
    }

    /* The second argument needs to be evaluated */
    expr *evaluated_arg;
    ret_code = eval(arg->cdr->car, &evaluated_arg);
    if (ret_code < 0) {
        printf("ERROR: BUILTIN: DEFINE: got error when evaluating the second argument: %d\n", ret_code);
        return ret_code;
    }

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
        if (curr->car->type != NUMBER) {
            printf("ERROR: BUILTIN: ADD: can only handle numbers but got %s\n",
                   type_str(curr->car->type));
            return -1;
        }
        acc += (int)curr->car->data;
    }
    expr *_out = expr_new(NUMBER, (uint64_t)acc, NULL, NULL);
    *out = _out;
    return 0;
}

int bi_sub(expr *arg, expr **out) {
    expr *curr = arg;
    for_each(curr) {
        if (curr->car == NULL) {
            printf("ERROR: BUILTIN: SUB: Argument to sub had a car that was NULL\n");
            return -1;
        }
        if (curr->car->type != NUMBER) {
            printf("ERROR: BUILTIN: SUB: can only handle numbers but got %s\n",
                   type_str(curr->car->type));
            return -1;
        }
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

    expr *_out = expr_new(NUMBER, acc, NULL, NULL);
    *out = _out;
    return 0;
}

int bi_cons(expr *arg, expr **out) {
    unsigned int size = list_length(arg);
    if (size != 2) {
        printf("ERROR: BUILTIN: CONS: need exactly two arguments, but got %d\n", size);
        return -1;
    }

    *out = expr_cons(arg->car, arg->cdr->car);
    return 0;
}


int bi_car(expr *arg, expr **out) {
    if (arg == NULL) {
        printf("WARNING: BUILTIN: CAR: got NULL as argument, returning NULL\n");
        *out = NULL;
        return 0;
    }
    if (arg->car == NULL) {
        printf("ERROR: BUILTIN: CAR: got an argument with no car field.\n");
        return -1;
    }
    if (arg->car->type != CONS) {
        printf("ERROR: BUILTIN: CAR: can only handle cons but got %s\n",
               type_str(arg->car->type));
        return -1;
    }
    /* Car is the first argument to the car builtin, and since
       the first argument is a cons cell, we need the car of that
       as well */
    *out = arg->car->car;
    return 0;
}

int bi_cdr(expr *arg, expr **out) {
    if (arg == NULL) {
        printf("WARNING: BUILTIN: CDR: got NULL as argument, returning NULL\n");
        *out = NULL;
        return 0;
    }
    if (arg->car == NULL) {
        printf("ERROR: BUILTIN: CDR: got an argument with no car field.\n");
        return -1;
    }
    if (arg->car->type != CONS) {
        printf("ERROR: BUILTIN: CDR: can only handle numbers but got %s\n",
               type_str(arg->car->type));
        return -1;
    }
    *out = arg->car->cdr;
    return 0;
}

int bi_progn(expr *arg, expr **out) {
    if (arg == NULL) {
        printf("WARNING: BUILTIN: PROGN: got NULL as argument, returning NULL\n");
        *out = NULL;
        return 0;
    }
    int ret_code;
    expr *curr_arg = arg, *_out;
    /* TODO: Probably can use the for_each macro here? */
    while (!list_end(curr_arg)) {
        ret_code = eval(curr_arg->car, &_out);
        if (ret_code < 0) {
            printf("ERROR: BUILTIN: PROGN: got error when evaluating an expression\n");
            return ret_code;
        }
        curr_arg = curr_arg->cdr;
    }
    *out = _out;
    return 0;
}

int bi_if(expr *arg, expr **out) {
    int ret_code;
    unsigned int arg_count = list_length(arg);
    if (arg_count != 3) {
        printf("ERROR: BUILTIN: IF: needs exactly three arguments, but got %d \n", arg_count);
        return -1;
    }

    expr *_eval;
    ret_code = eval(arg->car, &_eval);
    if (ret_code < 0) {
        printf("ERROR: BUILTIN: IF: got error when evaluating first argument\n");
        return ret_code;
    }
    if (expr_is_true(_eval)) {
        ret_code = eval(arg->cdr->car, out);
        if (ret_code < 0) {
            printf("ERROR: BUILTIN: IF: got error when evaluating second argument\n");
            return ret_code;
        }
    } else {
        ret_code = eval(arg->cdr->cdr->car, out);
        if (ret_code < 0) {
            printf("ERROR: BUILTIN: IF: got error when evaluating third argument\n");
            return ret_code;
        }
    }

    return 0;
}

int bi_print(expr *arg, expr **out) {
    int ret_code;
    expr *curr = arg;
    for_each(curr) {
        ret_code = expr_print(curr->car);
        if (ret_code < 0) {
            printf("ERROR: BUILTIN: PRINT: got error when printing expr\n");
            return -1;
        }
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
            val = expr_is_equal(prev->car, curr->car);
            if (val == -1) {
                printf("ERROR: BUILTIN: EQUAL: error when checking for equality\n");
                return -1;
            }
            if (val == 0) {
                break;
            }
            prev = curr;
        }
    }

    expr* new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    *out = new_expr;
    return 0;
}

int bi_gt(expr *arg, expr **out) {
    int arg_length = list_length(arg);
    if (arg_length != 2) {
        printf("ERROR: BUILTIN: GT: needs exactly two arguments\n");
        return -1;
    }

    int val = expr_gt_lt(arg->car, arg->cdr->car, 1);
    expr* new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    *out = new_expr;
    return 0;
}

int bi_lt(expr *arg, expr **out) {
    int arg_length = list_length(arg);
    if (arg_length != 2) {
        printf("ERROR: BUILTIN: LT: needs exactly two arguments\n");
        return -1;
    }

    int val = expr_gt_lt(arg->car, arg->cdr->car, 0);
    expr* new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    *out = new_expr;
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
            ret_code = eval(curr->car, &_eval);
            if (ret_code < 0) {
                printf("ERROR: BUILTIN: AND: builtin got error when evaluating expression\n");
                return ret_code;
            }
            val = expr_is_true(_eval);
            if (val == -1) {
                printf("ERROR: BUILTIN: AND: builtin got error when checking if expr is true\n");
                return -1;
            }
            if (val == 0) {
                break;
            }
        }
    }

    expr* new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    *out = new_expr;
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
            ret_code = eval(curr->car, &_eval);
            if (ret_code < 0) {
                printf("ERROR: BUILTIN: OR: builtin got error when evaluating expression\n");
                return -1;
            }
            val = expr_is_true(_eval);
            if (val == -1) {
                printf("ERROR: BUILTIN: OR: builtin got error when checking if expr is true\n");
                return -1;
            }
            if (val == 1) {
                break;
            }
        }
    }

    expr* new_expr = expr_new(BOOLEAN, (uint64_t)val, NULL, NULL);
    *out = new_expr;
    return 0;
}

int bi_quote(expr *arg, expr **out) {
    unsigned int arg_length = list_length(arg);
    if (arg_length != 1) {
        printf("ERROR: BUILTIN: QUOTE: only accepts exactly one argument, but got %d\n", arg_length);
        return -1;
    }
    *out = arg->car;
    return 0;
}

int bi_quasiquote(expr *arg, expr **out) {
    int ret_code;
    unsigned int arg_length = list_length(arg);
    if (arg_length != 1) {
        printf("ERROR: BUILTIN: QUASIQUOTE: only accepts exactly one argument, but got %d\n", arg_length);
        return -1;
    }
    expr *copy;
    ret_code = expr_copy(arg->car, &copy);
    if (ret_code < 0) {
        printf("ERROR: BUILTIN: QUASIQUOTE: copying expression\n");
        return ret_code;
    }
    ret_code = quasiquote_eval(&copy);
    if (ret_code < 0) {
        printf("ERROR: BUILTIN: QUASIQUOTE: running quasiquote eval\n");
        return 0;
    }
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
    unsigned int arg_length = list_length(arg);
    if (arg_length != 2) {
        printf("ERROR: BUILTIN: DEFMACRO: only accepts exactly one argument, but got %d\n", arg_length);
        return -1;
    }

    expr *curr_arg = arg->car;
    for_each(curr_arg) {
        if (curr_arg->car->type != SYMBOL) {
            printf("ERROR: BUILTIN: DEFMACRO: expects all arguments to be symbols, but found one that was of type %s\n",
                   type_str(curr_arg->car->type));
            return -1;
        }
    }

    char *buf = my_malloc(TOKEN_STR_MAX_LEN);
    char *macro_name = (char *)(arg->car->car->data);
    strcpy(buf, macro_name);

    symbol *new_sym = symbol_create(buf, MACRO, arg);

    symbol_add(new_sym);
    *out = arg;
    return 0;
}

int bi_macroexpand(expr *arg, expr **out) {
    unsigned int arg_length = list_length(arg);
    if (arg_length != 1) {
        printf("ERROR: BUILTIN: MACROEXPAND: only accepts exactly one argument, but got %d\n", arg_length);
        return -1;
    }

    symbol *sym = symbol_find((char *) arg->car->car->data);
    if (sym == NULL) {
        printf("ERROR: BUILTIN: MACROEXPAND: Couldnt find macro %s\n", (char *) arg->car->data);
        return -1;
    }

    expr *macro_expand;
    int func_inv_res = function_invocation(sym, arg->car->cdr, &macro_expand);
    if (func_inv_res < 0) {
        printf("ERROR: BUILTIN: MACROEXPAND: Got error when expanding macro %d\n", func_inv_res);
        return func_inv_res;
    }

    *out = macro_expand;
    return 0;
}
