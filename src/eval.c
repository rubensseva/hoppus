#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "parser.h"
#include "expr.h"
#include "symbol.h"
#include "config.h"
#include "memory.h"
#include "builtins.h"
#include "list.h"


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
int function_invocation(symbol *sym, expr *args, expr **res) {
    /* The parameters to the defun invocation that defined this function */
    expr *defun_params = sym->e;
    /* The parameters that the function that should be invoked, first entry
       is the function name */
    expr *function_params = defun_params->car;
    expr *name = defun_params->car->car;
    /* The forms representing the function logic */
    expr *forms = defun_params->cdr;

    /* Go to the next function param to skip the function name */
    expr *curr_param = function_params->cdr;
    expr *curr_arg = args;
    /* Would preferably use the for_each() macro here, but since
       we need to iterate through two lists, we need a custom loop */
    /* TODO: Consider getting the size once, then iterating, to
       avoid continually calculating list end */
    while (!list_end(curr_arg) || !list_end(curr_param)) {
        if (list_end(curr_arg) || list_end(curr_param)) {
            printf("ERROR: Mismatch between defun params and given function arguments for function: %s\n",
                    (char *)name->data);
            return -1;
        }
        /* The value of the symbol is in the args for the current
            procedure being handled. This value can be anything, so we
            need to evaluate. */
        expr *new_e = NULL;
        int eval_res = eval(curr_arg->car, &new_e);
        if (eval_res < 0) {
            printf("EVAL: ERROR: Function invocation error when evaluating arguments for symbols \n");
            return eval_res;
        }
        char *sym_name = (char *)curr_param->car->data;
        symbol *new_sym = symbol_create(sym_name, VARIABLE, new_e);
        symbol_add(new_sym);
        curr_param = curr_param->cdr;
        curr_arg = curr_arg->cdr;
    }
    /* After all the symbols are added, evaluate the function logic.
       There might be several lists of function logic, the return
       value will be the last of these */
    expr *curr_form = forms;
    while (!list_end(curr_form)) {
        int eval_res = eval(curr_form->car, res);
        if (eval_res < 0) {
            printf("EVAL: ERROR: Function invocation error when invoking function \n");
            return eval_res;
        }
        curr_form = curr_form->cdr;
    }
    /* After the function is evaluated, remove the symbols */
    curr_param = function_params->cdr;
    while (!list_end(curr_param)) {
        int sym_res = symbol_remove_name((char *)curr_param->car->data);
        if (sym_res < 0) {
            printf("WARNING: Unable to remove sumbol %s\n",
                   (char *)curr_param->car->data);
            return sym_res;
        }
        curr_param = curr_param->cdr;
    }
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
            printf("Got unknown expr type with printing\n");
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
        printf("EVAL: WARNING: Eval got NULL, returning NULL\n");
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
            printf("EVAL: WARNING: Found symbol with no value: %s. There is probably something wrong.\n",
                   (char *)e->data);
            *res = e;
            return 0;
        }
        case CONS:;
            expr *proc = e, *fun = proc->car, *arg = e->cdr;
            symbol *sym;
            if (proc == NULL || proc->car == NULL) {
                printf("EVAL: ERROR: expr was NULL when evaluating cons cell \n");
                return -1;
            }
            if (proc->car->type != SYMBOL) {
                *res = proc;
                return 0;
            }
            /* Here we can invoce special operators, which should not have their arguments evaluated */
            sym = symbol_find((char *)(fun->data));
            if (sym == NULL) {
                printf("EVAL: ERROR: Trying invoke function, but symbol was null\n");
                return -1;
            }
            if (sym->type == VARIABLE) {
                printf("EVAL: ERROR: Cannot use variable %s as function\n", (char *)sym->e->car->data);
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
                    printf("EVAL: ERROR: Attempting to exec special operator, but the symbol was not of type builtin\n");
                    return -1;
                }
                int bi_res = sym->builtin_fn(arg, res);
                if (bi_res == -1) {
                    printf("EVAL: ERROR: Builtin function encountered error\n");
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
                    printf("EVAL: ERROR: Got error when evaluating arguments\n");
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
                    printf("EVAL: ERROR: Builtin function encountered error\n");
                    return -1;
                }
                return 0;
            }
            if (sym->type == FUNCTION) {
                int func_inv_res = function_invocation(sym, first_cons, res);
                if (func_inv_res < 0) {
                    printf("EVAL: ERROR: Got error when invocing function %d\n", func_inv_res);
                    return func_inv_res;
                }
            } else if (sym->type == VARIABLE) {
                printf("EVAL: ERROR: Cant use variable %s as a function\n", (char*)fun->data);
                return -1;
            }
            return 0;
        default:
            printf("EVAL: ERROR: Got unknown type: %d\n", e->type);
            return -1;
    }
}
