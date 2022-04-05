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


int quasiquote_eval(expr **arg) {
    if (arg == NULL || *arg == NULL)
        return 0;
    int ret_code;
    switch((*arg)->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
        case SYMBOL:;
            return 0;
        case CONS:;
            if ((*arg)->car && (*arg)->car->type == SYMBOL) {
                symbol *sym = symbol_find((char *)((*arg)->car->data));
                if (sym && strcmp(sym->name, COMMA_STR) == 0) {
                    expr *tmp;
                    ret_code = eval((*arg)->cdr->car, &tmp);
                    if (ret_code < 0) {
                        printf("ERROR: EVAL: QUASIQUOTE_EVAL: Evaluating cdr of comma\n");
                        return ret_code;
                    }
                    (*arg) = tmp;
                    return 0;
                }
            }
            ret_code = quasiquote_eval(&((*arg)->car));
            if (ret_code < 0) {
                printf("ERROR: EVAL: QUASIQUOTE_EVAL: Recursively running quasiquote_eval on car\n");
                return ret_code;
            }
            ret_code = quasiquote_eval(&((*arg)->cdr));
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
   Add parameters as symbols, with matching args.

   args and params must be lists where each element in params has a corresponding
   element in args.
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
   Handles an invocation of a function.

   This function a little complex by the fact that we are dealing with two
   types of arguments, the arguments to the call to defun when the function was
   defined, and the arguments to this specific invocation of the function.

   When the function is defined with a call to defun, there are two arguments,
   where both of these arguments are lists. The first argument is a list of names, where
   the first entry in this list is the function name, and the rest is the names of
   the function arguments. The second argument to defun is a list which describes the
   function logic to be evaluated when the function should be invoked.

   The other type of arguments are the values passed to the function at the time
   of its invocation. These are values that should be bound to the corresponding
   symbol names


      1.      2.       3.
   (defun (add x y) (+ x y))

     4.  5.
   (add 1 2)

   1: The call to defun
   2: The first argument to defun, a list containing the name of the function and the name of the
       function arguments
   3: The second argument to defun, a list describing the function logic.
   4: An invocation of the "add" function, as defined in the above defun.
   5: The function arguments to the invocation of "add".


   Parameters:
       sym: The symbol that represents the defun to be invoced.
       invocation: Linked list of expression containing data for this invocation. The
                   length of the arguments in this linked list must match the length
                   of the arguments in sym.
       res: Output
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


int print_expr_tree(expr *e){
    if (e == NULL) {
        printf("nil");
        return 0;
    }
    switch(e->type) {
        case NUMBER:
            printf("%d", (int)e->data);
            return 0;
        case CHAR:
            printf("'%c'", (char)e->data);
            return 0;
        case SYMBOL:
            printf("%s", (char *)e->data);
            return 0;
        case CONS:;
            printf("(");
            /* We could use list_end() here, but in order for this
               to work for all kinds of cons cells, we need only check
               for curr */
            print_expr_tree(e->car);
            printf(" ");
            print_expr_tree(e->cdr);
            printf(")");
            return 0;
        case BOOLEAN:
            if (e->data) {
                printf("true");
            } else {
                printf("false");
            }
            return 0;
        default:
            printf("ERROR: EVAL: PRINT: Got unknown expr type\n");
            return -1;
    }
}

int print_expr(expr *e) {
    printf("\n");
    print_expr_tree(e);
    printf("\n");
    return 0;
}


int eval(expr *e, expr **res) {
    if (e == NULL) {
        printf("WARNING: EVAL: Eval got NULL, returning NULL\n");
        *res = NULL;
        return 0;
    }
    switch (e->type) {
        case NUMBER:
        case CHAR:
        case BOOLEAN:
            *res = e;
            return 0;
        case SYMBOL:;
        {
            symbol *sym = symbol_find((char *)e->data);
            if (sym != NULL) {
                /* If its a builtin, the function is defined in C, so we
                   dont care about the expression. So lets just return the
                   given expression in that case. */
                if (sym->type == BUILTIN) {
                    *res = e;
                    return 0;
                }
                *res = sym->e;
                return 0;
            }
            printf("WARNING: EVAL: Found symbol with no value: %s. There is probably something wrong.\n",
                   (char *)e->data);
            *res = e;
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
                *res = proc;
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
                *res = evald;
                return 0;
            }
            if (sym->is_special_operator) {
                if (sym->type != BUILTIN) {
                    printf("ERROR: EVAL: Attempting to exec special operator, but the symbol was not of type builtin\n");
                    return -1;
                }
                int bi_res = sym->builtin_fn(arg, res);
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
                int bi_res = sym->builtin_fn(first_cons, res);
                if (bi_res == -1) {
                    printf("ERROR: EVAL: Builtin function encountered error\n");
                    return -1;
                }
                return 0;
            }
            if (sym->type == FUNCTION) {
                int func_inv_res = function_invocation(sym, first_cons, res);
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
