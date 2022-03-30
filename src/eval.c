#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ir.h"
#include "symbol.h"
#include "config.h"
#include "memory.h"
#include "builtins.h"

expr *eval(expr *e);


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
int defun(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to cdr \n");
        return -1;
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
int define(expr *arg, expr **res) {
    if (arg == NULL || arg->cdr == NULL || arg->cdr->car == NULL || arg->cdr->cdr != NULL) {
        printf("ERROR: Define needs exactly two arguments\n");
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
*/
expr *function_invocation(symbol *sym, expr *invocation_values) {
    /* The two defun args (the list of arguments, and list of function logic) is
        in the data field of the defun symbol */
    expr *defun_args = sym->e;
    /* The function arguments for the defun is contained in the ->data field in
       the first defun_args entry */
    expr *defun_function_args = defun_args->car;
    /* The name of the defun is in the first function argument */
    expr *defun_name = defun_args->car->car;
    /* The function logic is contained in the second defun arguments */
    expr *defun_function_logic = defun_args->cdr;

    /* Go to the next function arg to skip the function name */
    expr *curr_defun_function_arg = defun_function_args->cdr;
    expr *curr_invocation_value = invocation_values;
    while (curr_invocation_value || curr_defun_function_arg) {
        if (!curr_invocation_value || !curr_defun_function_arg) {
            printf("ERROR: Mismatch between defun and given function arguments for function: %s\n",
                    (char *)defun_name->data);
            return NULL;
        }
        /* The value of the symbol is in the args for the current
            procedure being handled. This value can be anything, so we
            need to evaluate. */
        expr *new_e = eval(curr_invocation_value->car);
        char *name = (char *)curr_defun_function_arg->car->data;
        symbol *new_symbol = symbol_create(name, VARIABLE, new_e);
        symbol_add(new_symbol);
        curr_defun_function_arg = curr_defun_function_arg->cdr;
        curr_invocation_value = curr_invocation_value->cdr;
    }
    /* After all the symbols are added, evaluate.
       There might be several lists of function logic, the return
       value will be the last of these */
    expr *curr_defun_function_logic = defun_function_logic;
    expr *res;
    while (curr_defun_function_logic) {
        res = eval(curr_defun_function_logic->car);
        curr_defun_function_logic = curr_defun_function_logic->cdr;
    }
    /* After the function is evaluated, remove the symbols */
    curr_defun_function_arg = defun_function_args->cdr;
    while (curr_defun_function_arg) {
        if (symbol_remove_name((char *)curr_defun_function_arg->car->data) == -1) {
            printf("WARNING: Unable to remove sumbol %s\n",
                   (char *)curr_defun_function_arg->car->data);
        }
        curr_defun_function_arg = curr_defun_function_arg->cdr;
    }

    return res;
}

expr *free_tree(expr *e) {
    switch (e->type) {
        case ROOT:
            free_tree((expr *)e->data);
            my_free(e);
            break;
        case NUMBER:
            my_free(e);
            break;
        case SYMBOL:;
            /* symbol *sym = symbol_find((char *)e->data); */
            /* if (sym != NULL) { */
            /*     my_free(sym); */
            /* } */
            /* The data field contains a string, the name of the symbol, so we need to
               my_free that as well */
            my_free((char *)e->data);
            my_free(e);
            break;
        case CONS:;
            if (e->cdr)
                free_tree(e->cdr);
            if (e->car)
                free_tree(e->car);
            my_free(e);
            break;

        case PROC_SYMBOL:;
            my_free((char *)e->data);
            my_free(e);
            break;

        default:
            printf("no action\n");
            return 0;
    }
    return 0;
}

expr *eval(expr *e) {
    switch (e->type) {
        case ROOT:
            return eval((expr *)e->data);
        case NUMBER:
            return e;
        case SYMBOL:;
        {
            symbol *sym = symbol_find((char *)e->data);
            if (sym != NULL) {
                /* If its a builtin, the function is defined in C, so we
                   dont care about the expression. So lets just return the
                   given expression in that case. */
                if (sym->type == BUILTIN) {
                    return e;
                }
                return sym->e;
            }
            printf("WARNING: Found symbol with no value: %s. There is probably something wrong.\n",
                   (char *)e->data);
            return e;
        }
        case CONS:;
            expr *proc = e;
            expr *fun = proc->car;
            expr *arg = e->cdr;
            expr *res;

            if (proc == NULL || proc->car == NULL) {
                printf("ERROR: expr was NULL when evaluating cons cell \n");
                return NULL;
            }
            if (proc->car->type != SYMBOL) {
                printf("ERROR: expected car of cons cell to be a symbol, but it wasnt \n");
                return NULL;
            }

            /* If this is a defun, we should not evaluate the arguments */
            if (strcmp((char *)(fun->data), "defun") == 0) {
                defun(arg, &res);
                /* TODO: Maybe need to free args here? */
                return res;
            }
            /* If this is a define, we should only evaluate one of the arguments */
            if (strcmp((char *)(fun->data), "define") == 0) {
                define(arg, &res);
                /* TODO: Maybe need to free args here? */
                return res;
            }

            /* Evaluate all arguments, and build a new list of those arguments */
            expr *curr_arg = arg, *curr_eval = NULL, *first_cons = NULL, *prev_cons = NULL;
            while (curr_arg) {
                /* TODO: arg->car should maybe be freed here, but we
                   cant free it if arg->car is for example a number, since then
                   arg->car = eval(arg->car) */
                curr_eval = eval(curr_arg->car);
                expr *new_cons = expr_cons(curr_eval, NULL);
                if (first_cons == NULL)
                    first_cons = new_cons;
                if (prev_cons != NULL)
                    prev_cons->cdr = new_cons;
                prev_cons = new_cons;
                curr_arg = curr_arg->cdr;
            }

            symbol *sym = symbol_find((char *)(fun->data));
            if (sym != NULL) {
                if (sym->type == BUILTIN) {
                    sym->builtin_fn(first_cons, &res);
                }
                if (sym->type == FUNCTION) {
                    res = function_invocation(sym, first_cons);
                } else if (sym->type == VARIABLE) {
                    printf("ERROR: Cant use variable %s as a function\n", (char*)fun->data);
                    return NULL;
                }
            }

            expr *free_arg = first_cons;
            while (free_arg) {
                expr *tmp = free_arg;
                free_arg = free_arg->cdr;
                /* free_tree(tmp); */
            }

            return res;
        default:
            printf("no action\n");
            return 0;
    }
}
