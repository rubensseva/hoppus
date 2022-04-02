#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "parser.h"
#include "symbol.h"
#include "config.h"
#include "memory.h"
#include "builtins.h"
#include "list.h"


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
int define(expr *arg, expr **res) {
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
    /* The arguments to the defun invocation that defined this function */
    expr *defun_args = sym->e;
    /* The function arguments to the funtcion, first entry is the function name */
    expr *function_args = defun_args->car;
    expr *name = defun_args->car->car;
    /* The forms representing the function logic */
    expr *function_logic = defun_args->cdr;

    /* Go to the next function arg to skip the function name */
    expr *curr_arg = function_args->cdr;
    expr *curr_val = invocation_values;
    while (!list_end(curr_val) || !list_end(curr_arg)) {
        if (list_end(curr_val) || list_end(curr_arg)) {
            printf("ERROR: Mismatch between defun and given function arguments for function: %s\n",
                    (char *)name->data);
            return NULL;
        }
        /* The value of the symbol is in the args for the current
            procedure being handled. This value can be anything, so we
            need to evaluate. */
        expr *new_e = eval(curr_val->car);
        char *sym_name = (char *)curr_arg->car->data;
        symbol *new_sym = symbol_create(sym_name, VARIABLE, new_e);
        symbol_add(new_sym);
        curr_arg = curr_arg->cdr;
        curr_val = curr_val->cdr;
    }
    /* After all the symbols are added, evaluate the function logic.
       There might be several lists of function logic, the return
       value will be the last of these */
    expr *curr_function_logic = function_logic;
    expr *res;
    while (!list_end(curr_function_logic)) {
        res = eval(curr_function_logic->car);
        curr_function_logic = curr_function_logic->cdr;
    }
    /* After the function is evaluated, remove the symbols */
    curr_arg = function_args->cdr;
    while (!list_end(curr_arg)) {
        if (symbol_remove_name((char *)curr_arg->car->data) == -1) {
            printf("WARNING: Unable to remove sumbol %s\n",
                   (char *)curr_arg->car->data);
        }
        curr_arg = curr_arg->cdr;
    }

    return res;
}

expr *free_tree(expr *e) {
    switch (e->type) {
        case NUMBER:
            my_free(e);
            break;
        case SYMBOL:;
            /* TODO: Consider finding the symbol in the symbol table and
               freeing it */
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
        default:
            printf("EVAL: WARNING: Unknown type when freeing an expr tree\n");
            return NULL;
    }
    return NULL;
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



expr *eval(expr *e) {
    switch (e->type) {
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
            printf("EVAL: WARNING: Found symbol with no value: %s. There is probably something wrong.\n",
                   (char *)e->data);
            return e;
        }
        case CONS:;
            expr *proc = e, *fun = proc->car, *arg = e->cdr, *res;
            if (proc == NULL || proc->car == NULL) {
                printf("EVAL: ERROR: expr was NULL when evaluating cons cell \n");
                return NULL;
            }
            /* TODO: Search for the symbol in the symbol table, and check what symbol type it is.
               Then you can differentiate between variables and functions */
            if (proc->car->type != SYMBOL) {
                return proc;
            }

            /* If this is a defun, we should not evaluate the arguments */
            if (strcmp((char *)(fun->data), "defun") == 0) {
                if (defun(arg, &res) == -1) {
                    printf("EVAL: ERROR: Defun\n");
                    return NULL;
                }
                return res;
            }
            /* If this is a define, we should only evaluate one of the arguments */
            if (strcmp((char *)(fun->data), "define") == 0) {
                if (define(arg, &res) == -1) {
                    printf("EVAL: ERROR: Define\n");
                    return NULL;
                }
                return res;
            }

            /* Evaluate all arguments, and build a new list of those arguments */
            expr *curr_arg = arg, *curr_eval = NULL, *first_cons = NULL, *prev_cons = NULL;
            while (!list_end(curr_arg)) {
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
                    int bi_res = sym->builtin_fn(first_cons, &res);
                    if (bi_res == -1) {
                        printf("EVAL: ERROR: Builtin function encountered error\n");
                        return NULL;
                    }
                }
                if (sym->type == FUNCTION) {
                    res = function_invocation(sym, first_cons);
                } else if (sym->type == VARIABLE) {
                    printf("EVAL: ERROR: Cant use variable %s as a function\n", (char*)fun->data);
                    return NULL;
                }
            }

            return res;
        default:
            printf("EVAL: ERROR: Got unknown type\n");
            return NULL;
    }
}
