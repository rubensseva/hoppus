#include <string1.h>
#include <builtins.h>
#include <parser.h>
#include <eval.h>
#include <list.h>
#include <expr.h>
#include <symbol.h>
#include <hoppus_memory.h>
#include <hoppus_config.h>
#include <hoppus_constants.h>
#include <hoppus_stdio.h>
#include <hoppus_link.h>

#include <stdint.h>

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
__USER_TEXT int bi_defun(expr *arg, expr **out) {
    if (list_length(arg) < 3) return NUMBER_OF_ARGUMENTS_ERROR;

    expr *curr = nth(1, arg);
    for_each(curr) {
        if (tar(curr) != SYMBOL) return TYPE_ERROR;

        if (strcmp1((char *)dar(curr), REST_ARGUMENTS_STR) == 0) {
            if (cdr(curr) == NULL) {
                hoppus_puts("ERROR: BUILTIN: DEFUN: expected a symbol after &rest keyword, but got nothing\n");
                return NUMBER_OF_ARGUMENTS_ERROR;
            }
        }
    }

    char *defun_name = (char *)(dar(arg));
    char *buf = my_malloc(strlen1(defun_name) + 1);
    strcpy1(buf, defun_name);

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
__USER_TEXT int bi_define(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    if (tar(arg) != SYMBOL) return TYPE_ERROR;

    /* The second argument needs to be evaluated */
    expr *evaluated_arg;
    if ((ret_code = eval(car(cdr(arg)), &evaluated_arg) < 0)) return ret_code;

    char *define_name = (char *)dar(arg);
    char *buf = my_malloc(strlen1(define_name) + 1);
    /* The name of the symbol exist directly as a string in the first argument. */
    strcpy1(buf, define_name);
    symbol *new_sym = symbol_create(buf, VARIABLE, evaluated_arg);

    symbol_add(new_sym);
    *out = evaluated_arg;
    return 0;
}

__USER_TEXT int bi_add(expr *arg, expr **out) {
    int acc = 0;
    expr *curr = arg;
    for_each(curr) {
        if (type(car(curr)) != NUMBER) return TYPE_ERROR;
        acc += (int)dar(curr);
    }
    expr *_out = expr_new_val(NUMBER, (uintptr_t)acc);
    *out = _out;
    return 0;
}

__USER_TEXT int bi_sub(expr *arg, expr **out) {
    expr *curr = arg;
    for_each(curr) {
        if (type(car(curr)) != NUMBER) return TYPE_ERROR;
    }

    int acc = 0, arg_length = list_length(arg);
    if (arg_length == 0) {
        acc = 0;
    } else if (arg_length == 1) {
        acc = (int)dar(arg) * -1;
    } else {
        acc = (int)dar(arg);
        expr *curr = cdr(arg);
        for_each(curr) {
            acc -= (int)dar(curr);
        }
    }
    *out = expr_new_val(NUMBER, acc);
    return 0;
}

__USER_TEXT int bi_mult(expr *arg, expr **out) {
    int acc = 1;
    if (list_length(arg) == 0) {
        acc = 0;
    } else {
        expr *curr = arg;
        for_each(curr) {
            if (tar(curr) != NUMBER) return TYPE_ERROR;
            acc *= (int)dar(curr);
        }
    }
    expr *_out = expr_new_val(NUMBER, (uintptr_t)acc);
    *out = _out;
    return 0;
}

__USER_TEXT int bi_div(expr *arg, expr **out) {
    expr *curr = arg;
    for_each(curr) {
        if (tar(curr) != NUMBER) return TYPE_ERROR;
    }
    int acc = 0, arg_length = list_length(arg);
    if (arg_length == 0) {
        acc = 0;
    } else if (arg_length == 1) {
        acc = (int)dar(arg);
    } else {
        acc = (int)dar(arg);
        expr *curr = cdr(arg);
        for_each(curr) {
            acc /= (int)dar(curr);
        }
    }
    *out = expr_new_val(NUMBER, acc);
    return 0;
}

__USER_TEXT int bi_cons(expr *arg, expr **out) {
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    *out = expr_new_cons(car(arg), car(cdr(arg)));
    return 0;
}


__USER_TEXT int bi_car(expr *arg, expr **out) {
    if (arg == NULL || car(arg) == NULL) {
        *out = NULL;
        return 0;
    }
    if (tar(arg) != CONS) return TYPE_ERROR;
    *out = car(car(arg));
    return 0;
}

__USER_TEXT int bi_cdr(expr *arg, expr **out) {
    if (arg == NULL || car(arg) == NULL) {
        *out = NULL;
        return 0;
    }
    if (tar(arg) != CONS) return TYPE_ERROR;
    *out = cdr(car(arg));
    return 0;
}

__USER_TEXT int bi_progn(expr *arg, expr **out) {
    if (arg == NULL) {
        *out = NULL;
        return 0;
    }
    int ret_code;
    expr *curr = arg, *_out;
    for_each(curr) {
        if ((ret_code = eval(car(curr), &_out)) < 0) return ret_code;
    }
    *out = _out;
    return 0;
}

__USER_TEXT int bi_cond(expr *arg, expr **out) {
    int ret_code;
    expr *curr = arg;
    for_each(curr) {
        if (!car(curr) || !cdr(car(curr)))
            return INVALID_FORM_ERROR;
        expr *pred = car(car(curr)), *form = car(cdr(car(curr)));
        expr *evald;
        if ((ret_code = eval(pred, &evald)) < 0) return ret_code;
        if (expr_is_true(evald)) {
            if ((ret_code = eval(form, out)) < 0) return ret_code;
            return 0;
        }
    }
    *out = NULL;
    return 0;
}

__USER_TEXT int bi_print(expr *arg, expr **out) {
    int ret_code;
    expr *curr = arg;
    for_each(curr) {
        if ((ret_code = expr_print(car(curr))) < 0) return ret_code;
    }
    *out = car(arg);
    return 0;
}

__USER_TEXT int bi_equal(expr *arg, expr **out) {
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
            if ((val = expr_is_equal(car(prev), car(curr))) < 0) return val;
            if (val == 0)
                break;
            prev = curr;
        }
    }
    *out = expr_new_val(BOOLEAN, (uintptr_t)val);
    return 0;
}

__USER_TEXT int bi_gt(expr *arg, expr **out) {
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    int val = expr_gt_lt(car(arg), car(cdr(arg)), 1);
    *out = expr_new_val(BOOLEAN, (uintptr_t)val);
    return 0;
}

__USER_TEXT int bi_lt(expr *arg, expr **out) {
    if (list_length(arg) != 2) return NUMBER_OF_ARGUMENTS_ERROR;
    int val = expr_gt_lt(car(arg), car(cdr(arg)), 0);
    *out = expr_new_val(BOOLEAN, (uintptr_t)val);
    return 0;
}

__USER_TEXT int bi_and(expr *arg, expr **out) {
    int val, ret_code, arg_length = list_length(arg);
    if (arg_length == 0) {
        val = 0;
    } else {
        val = 1;
        expr *curr = arg;
        for_each(curr) {
            expr *_eval;
            if ((ret_code = eval(car(curr), &_eval)) < 0) return ret_code;
            if ((val = expr_is_true(_eval)) < 0) return val;
            if (val == 0) break;
        }
    }
    *out = expr_new_val(BOOLEAN, (uintptr_t)val);
    return 0;
}

__USER_TEXT int bi_or(expr *arg, expr **out) {
    int val, ret_code, arg_length = list_length(arg);
    if (arg_length == 0) {
        val = 0;
    } else {
        val = 0;
        expr *curr = arg;
        for_each(curr) {
            expr *_eval;
            if ((ret_code = eval(car(curr), &_eval)) < 0) return ret_code;
            if ((val = expr_is_true(_eval)) < 0) return val;
            if (val == 1) break;
        }
    }

    expr* new_expr = expr_new_val(BOOLEAN, (uintptr_t)val);
    *out = new_expr;
    return 0;
}

__USER_TEXT int bi_quote(expr *arg, expr **out) {
    if (list_length(arg) != 1) return NUMBER_OF_ARGUMENTS_ERROR;
    *out = car(arg);
    return 0;
}

__USER_TEXT int bi_quasiquote(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 1) return NUMBER_OF_ARGUMENTS_ERROR;
    expr *copy;
    if ((ret_code = expr_copy(car(arg), &copy)) < 0) return ret_code;
    if ((ret_code = quasiquote_eval(&copy)) < 0) return ret_code;
    *out = copy;
    return 0;
}

/** If this builtin is triggered as part of normal evaluation, something is wrong.
   All comma signs should be within a quasiquote, and will be handled by quasitquote
   without evaluating the comma symbol itself */
__USER_TEXT int bi_comma(expr *arg, expr **out) {
    hoppus_puts("ERROR: BUILTIN: COMMA: encountered comma outside quasiquote\n");
    return -1;
}
/** If this builtin is triggered as part of normal evaluation, something is wrong.
   All comma-at signs should be within a quasiquote, and will be handled by quasitquote
   without evaluating the comma-at symbol itself */
__USER_TEXT int bi_comma_at(expr *arg, expr **out) {
    hoppus_puts("ERROR: BUILTIN: COMMA-at: encountered comma-at outside quasiquote\n");
    return -1;
}

__USER_TEXT int bi_defmacro(expr *arg, expr **out) {
    if (list_length(arg) < 3) return NUMBER_OF_ARGUMENTS_ERROR;
    expr *curr = car(cdr(arg));
    for_each(curr) {
        if (tar(curr) != SYMBOL) return TYPE_ERROR;
    }
    char *macro_name = (char *)dar(arg);
    char *buf = my_malloc(strlen1(macro_name) + 1);
    strcpy1(buf, macro_name);
    symbol_add(symbol_create(buf, MACRO, arg));
    *out = arg;
    return 0;
}

__USER_TEXT int bi_macroexpand(expr *arg, expr **out) {
    int ret_code;
    if (list_length(arg) != 1) return NUMBER_OF_ARGUMENTS_ERROR;
    symbol *sym = symbol_find((char *) dar(car(arg)));
    if (sym == NULL) return UNBOUND_SYMBOL_NAME_ERROR;
    expr *macro_expand;
    if ((ret_code = function_invocation(sym, cdr(car(arg)), &macro_expand)) < 0)
        return ret_code;
    *out = macro_expand;
    return 0;
}
