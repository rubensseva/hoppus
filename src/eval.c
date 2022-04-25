#include <stdlib.h>
#include <USER_stdio.h>
#include <string1.h>

#include <gc.h>
#include <eval.h>
#include <parser.h>
#include <expr.h>
#include <symbol.h>
#include <clisp_config.h>
#include <constants.h>
#include <clisp_memory.h>
#include <builtins.h>
#include <list.h>


/** @brief Scans an expression tree for comma or comma-at signs, then takes
    appropriate action.

    @param e The expression tree to scan. It also acts as output and will
    be modified in-place.
    @return Status code
*/
int quasiquote_eval(expr **e) {
    int ret_code;
    if (e == NULL || *e == NULL)
        return 0;
    switch(type(*e)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
        case SYMBOL:;
            return 0;
        case CONS:;
            int handled_car = 0;
            if (car(*e) && tar(*e) == CONS && car(car(*e)) && tar(car(*e)) == SYMBOL) {
                symbol *sym = symbol_find((char *)(dar(car(*e))));
                if (sym && strcmp1(sym->name, COMMA_STR) == 0) {
                    expr *evald;
                    ret_code = eval(car(cdr(car(*e))), &evald);
                    if (ret_code < 0) {
                        printf("ERROR: EVAL: QUASIQUOTE_EVAL: Evaluating cdr of comma\n");
                        goto error;
                    }
                    set_car(*e, evald);
                    handled_car = 1;
                }
                if (sym && strcmp1(sym->name, COMMA_AT_STR) == 0) {
                    expr *evald;
                    ret_code = eval(car(cdr(car(*e))), &evald);
                    if (ret_code < 0) {
                        printf("ERROR: EVAL: QUASIQUOTE_EVAL: Evaluating cdr of comma-at\n");
                        goto error;
                    }
                    /* Splice */
                    expr *old_cdr = cdr(*e);
                    *e = evald;
                    while (!list_end(cdr(evald))) {evald = cdr(evald);};
                    set_cdr(evald, old_cdr);
                    handled_car = 1;
                }
            }
            if (!handled_car) {
                expr *tmp = car(*e);
                ret_code = quasiquote_eval(&tmp);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: QUASIQUOTE_EVAL: Recursively running quasiquote_eval on car\n");
                    goto error;
                }
                set_car(*e, tmp);
            }
            expr *tmp = cdr(*e);
            ret_code = quasiquote_eval(&tmp);
            if (ret_code < 0) {
                printf("ERROR: EVAL: QUASIQUOTE_EVAL: Recursively running quasiquote_eval on cdr\n");
                goto error;
            }
            set_cdr(*e, tmp);
            return 0;
        default:
            printf("ERROR: EVAL: QUASIQUOTE_EVAL: Got unknown type: %d\n", type(*e));
            ret_code = -1;
            goto error;
    }

error:
    expr_print(*e);
    return ret_code;
}


/**
   @brief Adds a list of parameters as symbols

   The "params" and "args" parameters are lists that should match entry for entry,
   unless a &rest parameter is involved.

   @param args List of arguments, the values the symbols
   @param params List of parameters, the names of the symbols
   @return Status code
*/
int add_param_symbols(expr *params, expr *args) {
    expr *curr_arg = args, *curr_param = params,
        *rest_param = NULL, *rest_arguments = NULL, *curr_rest_argument = NULL;
    int is_rest = 0;
    /* Would preferably use the for_each() macro here, but since
       we need to iterate through two lists, we need a custom loop */
    /* TODO: Simplify the boolean logic here */
    while ((!is_rest && !list_end(curr_param)) || !list_end(curr_arg)) {
        if ((!is_rest && list_end(curr_param)) || list_end(curr_arg)) {
            printf("ERROR: EVAL: ADD_PARAM_SYMBOLS: mismatch between number of args and params\n");
            return NUMBER_OF_ARGUMENTS_ERROR;
        }

        if (!is_rest && strcmp1((char *)dar(curr_param), REST_ARGUMENTS_STR) == 0) {
            if (cdr(curr_param) == NULL) {
                printf("ERROR: EVAL: ADD_PARAM_SYMBOLS: found &rest keyword, but no argument after it\n");
                return -1;
            }
            is_rest = 1;
            rest_param = car(cdr(curr_param));
            rest_arguments = NULL;
            curr_rest_argument = NULL;
        }

        if (is_rest) {
            expr *new_cons = expr_new_cons(car(curr_arg), NULL);
            if (rest_arguments == NULL) {
                curr_rest_argument = new_cons;
                rest_arguments = curr_rest_argument;
            } else {
                set_cdr(curr_rest_argument, new_cons);
                curr_rest_argument = cdr(curr_rest_argument);
            }
        } else {
            char *sym_name = (char *)dar(curr_param);
            symbol *new_sym = symbol_create(sym_name, VARIABLE, car(curr_arg));
            symbol_add(new_sym);
            curr_param = cdr(curr_param);
        }
        curr_arg = cdr(curr_arg);
    }
    if (is_rest) {
        char *sym_name = (char *)data(rest_param);
        symbol *new_sym = symbol_create(sym_name, VARIABLE, rest_arguments);
        symbol_add(new_sym);
    }
    return 0;
}

int remove_param_symbols(expr *params) {
    expr *curr_param = params;
    for_each(curr_param) {
        if (strcmp1((char *)dar(curr_param), REST_ARGUMENTS_STR) == 0) {
            continue;
        }
        int sym_res = symbol_remove_name((char *)dar(curr_param));
        if (sym_res < 0) {
            printf("WARNING: EVAL: Unable to remove symbol %s\n",
                   (char *)dar(curr_param));
            return sym_res;
        }
    }
    return 0;
}

/**
   @brief Handles an invocation of a function.

   @param sym The symbol that represents the defun to be invoked. It contains
   the function forms to be executed, and the parameters of the function.
   @param args The arguments to this invokation
   @return status code
*/
int function_invocation(symbol *sym, expr *args, expr **out) {
    int ret_code;
    expr *defun_params = sym->e;
    expr *function_params = car(cdr(defun_params)), *name = car(defun_params),
        *forms = cdr(cdr(defun_params));;

    if ((ret_code = add_param_symbols(function_params, args)) < 0) return ret_code;

    /* Evaluate the function forms. The evaluation of the last form will be returned. */
    expr *curr_form = forms;
    while (!list_end(curr_form)) {
        int eval_res = eval(car(curr_form), out);
        if (eval_res < 0) {
            printf("ERROR: EVAL: FUNCTION_INVOKATION: error when evaluating function forms\n");
            return eval_res;
        }
        curr_form = cdr(curr_form);
    }

    remove_param_symbols(function_params);
    return 0;
}

int eval(expr *e, expr **out) {
    int ret_code;
    if (e == NULL) {
        printf("WARNING: EVAL: Eval got NULL, returning NULL\n");
        *out = NULL;
        return 0;
    }
    switch (type(e)) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            *out = e;
            return 0;
        case SYMBOL:;
        {
            symbol *sym = symbol_find((char *)data(e));
            if (sym != NULL) {
                /* If its a builtin, the function is defined in C, so we
                   dont care about the expression. So lets just return the
                   given expression in that case. */
                if (sym->type == BUILTIN) {
                    *out = e;
                    return 0;
                }
                expr *copy;
                if ((ret_code = expr_copy(sym->e, &copy)) < 0) return ret_code;
                *out = copy;
                return 0;
            }
            printf("ERROR: EVAL: couldnt find symbol: %s\n", (char *)data(e));
            ret_code = UNBOUND_SYMBOL_NAME_ERROR;
            goto error;
        }
        case CONS:;
            expr *proc = e, *fun = car(proc), *arg = cdr(e);
            symbol *sym;
            if (proc == NULL || car(proc) == NULL) {
                printf("ERROR: EVAL: expr was NULL when evaluating cons cell \n");
                ret_code = -1;
                goto error;
            }
            if (tar(proc) != SYMBOL) {
                *out = proc;
                return 0;
            }
            /* Here we can invoce special operators, which should not have their arguments evaluated */
            if ((sym = symbol_find((char *)(data(fun)))) == NULL) {
                printf("ERROR: EVAL: unable find function \"%s\"\n",
                       (char *)(data(fun)));
                ret_code = UNBOUND_SYMBOL_NAME_ERROR;
                goto error;
            }
            if (sym->type == VARIABLE) {
                printf("ERROR: EVAL: cannot use variable %s as function\n", sym->name);
                ret_code = TYPE_ERROR;
                goto error;
            }
            if (sym->type == MACRO) {
                expr *macro_expand;
                ret_code = function_invocation(sym, arg, &macro_expand);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: got error when expanding macro %d\n", ret_code);
                    goto error;
                }
                expr *evald;
                ret_code = eval(macro_expand, &evald);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: got error when evaluating expanded macro %d\n", ret_code);
                    expr_print(macro_expand);
                    goto error;
                }
                *out = evald;
                return 0;
            }
            if (sym->is_special_operator) {
                if (sym->type != BUILTIN) {
                    printf("ERROR: EVAL: attempting to exec special operator, but the symbol was not of type builtin\n");
                    ret_code = TYPE_ERROR;
                    goto error;
                }
                ret_code = sym->builtin_fn(arg, out);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: builtin function \"%s\" encountered an error\n", sym->name);
                    goto error;
                }
                return 0;
            }

            /* Evaluate all arguments, and build a new list of those arguments */
            expr *curr_arg = arg, *first_cons = NULL, *prev_cons = NULL;
            for_each(curr_arg) {
                expr *curr_eval;
                ret_code = eval(car(curr_arg), &curr_eval);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: got error when evaluating argument\n");
                    expr_print(curr_arg);
                    goto error;
                }
                expr *new_cons = expr_new_cons(curr_eval, NULL);
                if (first_cons == NULL)
                    first_cons = new_cons;
                if (prev_cons != NULL)
                    set_cdr(prev_cons, new_cons);
                prev_cons = new_cons;
            }

            /* Invoke the builtin or lisp function. We already found the symbol
               earlier when checking for special operators. */
            if (sym->type == BUILTIN) {
                ret_code = sym->builtin_fn(first_cons, out);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: builtin function \"%s\" encountered an error\n", sym->name);
                    goto error;
                }
                return 0;
            }
            if (sym->type == FUNCTION) {
                ret_code = function_invocation(sym, first_cons, out);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: function \"%s\" encountered an error\n", sym->name);
                    goto error;
                }
            } else if (sym->type == VARIABLE) {
                printf("ERROR: EVAL: Cant use variable %s as a function\n", (char*)data(fun));
                ret_code = -1;
                goto error;
            }
            return 0;
        default:
            printf("ERROR: EVAL: Got unknown type: %d\n", type(e));
            ret_code = -1;
            goto error;
    }

error:
    expr_print(e);
    return ret_code;
}
