#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "parser.h"
#include "expr.h"
#include "symbol.h"
#include "config.h"
#include "constants.h"
#include "memory.h"
#include "builtins.h"
#include "list.h"


/** @brief Scans an expression tree for comma or comma-at signs, then takes
    appropriate action.

    @param e The expression tree to scan. It also acts as output and will
    be modified in-place.
    @return Status code
*/
int quasiquote_eval(expr **e) {
    if (e == NULL || *e == NULL)
        return 0;
    int ret_code;
    switch((*e)->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
        case SYMBOL:;
            return 0;
        case CONS:;
            if ((*e)->car && (*e)->car->car && (*e)->car->car->type == SYMBOL) {
                symbol *sym = symbol_find((char *)((*e)->car->car->data));
                if (sym && strcmp(sym->name, COMMA_STR) == 0) {
                    expr *evald;
                    ret_code = eval((*e)->car->cdr->car, &evald);
                    if (ret_code < 0) {
                        printf("ERROR: EVAL: QUASIQUOTE_EVAL: Evaluating cdr of comma\n");
                        return ret_code;
                    }
                    (*e)->car = evald;
                    return 0;
                }
                if (sym && strcmp(sym->name, COMMA_AT_STR) == 0) {
                    expr *evald;
                    ret_code = eval((*e)->car->cdr->car, &evald);
                    if (ret_code < 0) {
                        printf("ERROR: EVAL: QUASIQUOTE_EVAL: Evaluating cdr of comma-at\n");
                        return ret_code;
                    }

                    /* Splice */
                    expr *old_cdr = (*e)->cdr;
                    *e = evald;
                    while (!list_end(evald->cdr)) {evald = evald->cdr;};
                    evald->cdr = old_cdr;

                    return 0;
                }
            }
            ret_code = quasiquote_eval(&((*e)->car));
            if (ret_code < 0) {
                printf("ERROR: EVAL: QUASIQUOTE_EVAL: Recursively running quasiquote_eval on car\n");
                return ret_code;
            }
            ret_code = quasiquote_eval(&((*e)->cdr));
            if (ret_code < 0) {
                printf("ERROR: EVAL: QUASIQUOTE_EVAL: Recursively running quasiquote_eval on cdr\n");
                return ret_code;
            }
            return 0;
        default:
            printf("ERROR: BUILTIN: QUASIQUOTE_EVAL: got unknown type\n");
            return -1;
    }

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
    /* TODO: Consider getting the size once, then iterating, to
       avoid continually calculating list end */
    /* TODO: Simplify the boolean logic here */
    while ((!is_rest && !list_end(curr_param)) || !list_end(curr_arg)) {
        if ((!is_rest && list_end(curr_param)) || list_end(curr_arg)) {
            printf("ERROR: EVAL: ADD_PARAM_SYMBOLS: Mismatch between number of args and params\n");
            return -1;
        }

        if (!is_rest && strcmp((char *)curr_param->car->data, REST_ARGUMENTS_STR) == 0) {
            if (curr_param->cdr == NULL) {
                printf("ERROR: EVAL: ADD_PARAM_SYMBOLS: Found &rest keyword, but no argument after it\n");
                return -1;
            }
            is_rest = 1;
            rest_param = curr_param->cdr->car;
            rest_arguments = NULL;
            curr_rest_argument = NULL;
        }

        if (is_rest) {
            expr *new_cons = expr_cons(curr_arg->car, NULL);
            if (rest_arguments == NULL) {
                curr_rest_argument = new_cons;
                rest_arguments = curr_rest_argument;
            } else {
                curr_rest_argument->cdr = new_cons;
                curr_rest_argument = curr_rest_argument->cdr;
            }
        } else {
            char *sym_name = (char *)curr_param->car->data;
            symbol *new_sym = symbol_create(sym_name, VARIABLE, curr_arg->car);
            symbol_add(new_sym);
            curr_param = curr_param->cdr;
        }
        curr_arg = curr_arg->cdr;
    }
    if (is_rest) {
        char *sym_name = (char *)rest_param->data;
        symbol *new_sym = symbol_create(sym_name, VARIABLE, rest_arguments);
        symbol_add(new_sym);
    }
    return 0;
}

int remove_param_symbols(expr *params) {
    expr *curr_param = params;
    for_each(curr_param) {
        if (strcmp((char *)curr_param->car->data, REST_ARGUMENTS_STR) == 0) {
            continue;
        }
        int sym_res = symbol_remove_name((char *)curr_param->car->data);
        if (sym_res < 0) {
            printf("WARNING: EVAL: Unable to remove symbol %s\n",
                (char *)curr_param->car->data);
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
    expr *defun_params = sym->e;
    expr *function_params = defun_params->car, *name = defun_params->car->car,
        *forms = defun_params->cdr;

    add_param_symbols(function_params->cdr, args);

    /* Evaluate the function forms. The evaluation of the last form will be returned. */
    expr *curr_form = forms;
    while (!list_end(curr_form)) {
        int eval_res = eval(curr_form->car, out);
        if (eval_res < 0) {
            printf("ERROR: EVAL: FUNCTION_INVOKATION: error when evaluating function forms\n");
            return eval_res;
        }
        curr_form = curr_form->cdr;
    }

    remove_param_symbols(function_params->cdr);
    return 0;
}

int eval(expr *e, expr **out) {
    if (e == NULL) {
        printf("WARNING: EVAL: Eval got NULL, returning NULL\n");
        *out = NULL;
        return 0;
    }
    switch (e->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            *out = e;
            return 0;
        case SYMBOL:;
        {
            symbol *sym = symbol_find((char *)e->data);
            if (sym != NULL) {
                /* If its a builtin, the function is defined in C, so we
                   dont care about the expression. So lets just return the
                   given expression in that case. */
                if (sym->type == BUILTIN) {
                    *out = e;
                    return 0;
                }
                expr *copy;
                int ret_code = expr_copy(sym->e, &copy);
                if (ret_code < 0) {
                    printf("ERROR: EVAL: Error when copying symbol");
                    return ret_code;
                }
                *out = copy;
                return 0;
            }
            printf("WARNING: EVAL: Couldnt find symbol: %s\n", (char *)e->data);
            *out = e;
            return 0;
        }
        case CONS:;
            expr *proc = e, *fun = proc->car, *arg = e->cdr;
            symbol *sym;
            if (proc == NULL || proc->car == NULL) {
                printf("ERROR: EVAL: expr was NULL when evaluating cons cell \n");
                return -1;
            }
            if (proc->car->type != SYMBOL) {
                *out = proc;
                return 0;
            }
            /* Here we can invoce special operators, which should not have their arguments evaluated */
            sym = symbol_find((char *)(fun->data));
            if (sym == NULL) {
                printf("ERROR: EVAL: Trying to invoke function, but couldnt find symbol %s\n",
                       (char *)(fun->data));
                return -1;
            }
            if (sym->type == VARIABLE) {
                printf("ERROR: EVAL: Cannot use variable %s as function\n", sym->name);
                return -1;
            }
            if (sym->type == MACRO) {
                expr *macro_expand;
                int func_inv_res = function_invocation(sym, arg, &macro_expand);
                if (func_inv_res < 0) {
                    printf("ERROR: EVAL: Got error when expanding macro %d\n", func_inv_res);
                    return func_inv_res;
                }
                expr *evald;
                int eval_res = eval(macro_expand, &evald);
                if (eval_res < 0) {
                    printf("ERROR: EVAL: Got error when evaluating expanded macro %d\n", eval_res);
                    return eval_res;
                }
                *out = evald;
                return 0;
            }
            if (sym->is_special_operator) {
                if (sym->type != BUILTIN) {
                    printf("ERROR: EVAL: Attempting to exec special operator, but the symbol was not of type builtin\n");
                    return -1;
                }
                int bi_res = sym->builtin_fn(arg, out);
                if (bi_res == -1) {
                    printf("ERROR: EVAL: Builtin function encountered error\n");
                    return -1;
                }
                return 0;
            }

            /* Evaluate all arguments, and build a new list of those arguments */
            expr *curr_arg = arg, *first_cons = NULL, *prev_cons = NULL;
            for_each(curr_arg) {
                expr *curr_eval;
                int eval_res = eval(curr_arg->car, &curr_eval);
                if (eval_res < 0) {
                    printf("ERROR: EVAL: Got error when evaluating arguments\n");
                    return eval_res;
                }
                expr *new_cons = expr_cons(curr_eval, NULL);
                if (first_cons == NULL)
                    first_cons = new_cons;
                if (prev_cons != NULL)
                    prev_cons->cdr = new_cons;
                prev_cons = new_cons;
            }

            /* Invoke the builtin or lisp function. We already found the symbol
               earlier when checking for special operators. */
            if (sym->type == BUILTIN) {
                int bi_res = sym->builtin_fn(first_cons, out);
                if (bi_res == -1) {
                    printf("ERROR: EVAL: Builtin function encountered error\n");
                    return -1;
                }
                return 0;
            }
            if (sym->type == FUNCTION) {
                int func_inv_res = function_invocation(sym, first_cons, out);
                if (func_inv_res < 0) {
                    printf("ERROR: EVAL: Got error when invocing function %d\n", func_inv_res);
                    return func_inv_res;
                }
            } else if (sym->type == VARIABLE) {
                printf("ERROR: EVAL: Cant use variable %s as a function\n", (char*)fun->data);
                return -1;
            }
            return 0;
        default:
            printf("ERROR: EVAL: Got unknown type: %d\n", e->type);
            return -1;
    }
}
