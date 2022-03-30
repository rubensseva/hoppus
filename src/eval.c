#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ir.h"
#include "symbol.h"
#include "config.h"
#include "memory.h"

expr *eval(expr *e);

int add(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to add \n");
        return -1;
    }
    uint64_t acc = 0;
    while (arg) {
        if (arg->type != NUMBER) {
            printf("TYPE ERROR: Add can only handle numbers\n");
            return -1;
        }
        acc += arg->data;
        arg = arg->next;
    }

    expr *_res = my_malloc(sizeof(expr));
    _res->type = NUMBER;
    _res->data = acc;
    _res->next = NULL;
    *res = _res;
    return 0;
}

int sub(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to sub \n");
        return -1;
    }
    if (arg->next == NULL) {
        if (arg->type != NUMBER) {
            printf("TYPE ERROR: Sub can only handle numbers\n");
            return -1;
        }
        return arg->data * (-1);
    }

    uint64_t acc = arg->data;
    arg = arg->next;
    while (arg) {
        if (arg->type != NUMBER) {
            printf("TYPE ERROR: Sub can only handle numbers\n");
            return -1;
        }
        acc -= arg->data;
        arg = arg->next;
    }
    expr *_res = my_malloc(sizeof(expr));
    _res->type = NUMBER;
    _res->data = acc;
    _res->next = NULL;
    *res = _res;
    return 0;
}

int cons(expr *arg, expr **res) {
    if (arg == NULL || arg->next == NULL || arg->next->next != NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Cons accepts exactly two arguments \n");
        return -1;
    }

    expr *cons = my_malloc(sizeof(expr));
    cons->type = CONS;

    expr *exprs = (expr *)my_malloc(2 * sizeof(expr));
    exprs[0].type = arg->type;
    exprs[0].data = arg->data;
    exprs[0].next = NULL;
    exprs[1].type = arg->next->type;
    exprs[1].data = arg->next->data;
    exprs[1].next = NULL;

    cons->data = (uint64_t) exprs;
    *res = cons;
    return 0;
}


int car(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to car \n");
        return -1;
    }
    if (arg->type != CONS) {
        printf("ERROR: Car can only handle cons cells\n");
        return -1;
    }
    *res = &((expr *)arg->data)[0];
    return 0;
}

int cdr(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to cdr \n");
        return -1;
    }
    if (arg->type != CONS) {
        printf("ERROR: Cdr can only handle cons cells\n");
        return -1;
    }

    *res = &((expr *)arg->data)[1];
    return 0;
}

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

   Illustraion, vertical lines are ->data field, horizontal lines are ->next field


   defun
    |
    |
    |
   args        -->         function logic
    |                              \
    |                               \
    |                                \
   f_name --> arg1 --> arg2         list of function logic

*/
int defun(expr *arg, expr **res) {
    if (arg == NULL) {
        /* TODO: Consider returning 0? */
        printf("ERROR: Nothing to cdr \n");
        return -1;
    }

    expr *new_e = my_malloc(sizeof(expr));
    new_e->type = DEFUN;
    new_e->next = NULL;
    new_e->data = (uint64_t)arg;

    symbol *new_sym = (symbol *) my_malloc(sizeof(symbol));
    char *buf = my_malloc(MAX_TOKEN_LENGTH);
    char *defun_name = (char *)(((expr *)(arg->data))->data);
    strcpy(buf, defun_name);

    new_sym->name = buf;
    new_sym->e = new_e;

    symbol_add(new_sym);
    *res = new_e;
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
    if (arg == NULL || arg->next == NULL || arg->next->next != NULL) {
        printf("ERROR: Define needs exactly two arguments\n");
        return -1;
    }

    /* The second argument needs to be evaluated */
    expr *evaluated_arg = eval(arg->next);

    expr *new_e = my_malloc(sizeof(expr));
    new_e->type = DEFINE;
    new_e->next = NULL;
    new_e->data = (uint64_t) evaluated_arg;

    symbol *new_sym = (symbol *) my_malloc(sizeof(symbol));
    char *buf = my_malloc(MAX_TOKEN_LENGTH);
    /* The name of the symbol exist directly as a string in the first argument. */
    strcpy(buf, (char *)arg->data);

    new_sym->name = buf;
    new_sym->e = new_e;

    symbol_add(new_sym);
    *res = new_e;
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
    expr *defun_args = (expr *)sym->e->data;
    /* The function arguments for the defun is contained in the ->data field in
       the first defun_args entry */
    expr *defun_function_args = (expr *)defun_args->data;
    /* The name of the defun is in the first function argument */
    expr *defun_name = defun_function_args;
    /* The function logic is contained in the second defun arguments */
    expr *defun_function = (expr *)defun_args->next->data;

    /* Go to the next function arg to skip the function name */
    expr *curr_defun_function_arg = defun_function_args->next;
    expr *curr_invocation_value = invocation_values;
    while (curr_invocation_value || curr_defun_function_arg) {
        if (!curr_invocation_value || !curr_defun_function_arg) {
            printf("ERROR: Mismatch between defun and given function arguments for function: %s\n",
                    (char *)defun_name->data);
            return NULL;
        }
        symbol *new_symbol = my_malloc(sizeof(symbol));
        /* The name of the symbol is in the defun args */
        new_symbol->name = (char *)curr_defun_function_arg->data;
        /* The value of the symbol is in the args for the current
            procedure being handled. This value can be anything, so we
            need to evaluate. */
        new_symbol->e = eval(curr_invocation_value);
        symbol_add(new_symbol);
        curr_defun_function_arg = curr_defun_function_arg->next;
        curr_invocation_value = curr_invocation_value->next;
    }
    /* After all the symbols are added, evaluate */
    expr *res = eval(defun_function);
    /* After the function is evaluated, remove the symbols */
    /* TODO: Implement shadowing of variables. As it is now, all
        variables must be unique, otherwise they are removed when an
        otherwise shadowed variable is removed */
    curr_defun_function_arg = defun_function_args->next;
    while (curr_defun_function_arg) {
        symbol_remove_name((char *)curr_defun_function_arg->data);
        curr_defun_function_arg = curr_defun_function_arg->next;
    }

    return res;
}

expr *free_tree(expr *e) {
    switch (e->type) {
        case ROOT:
            free_tree((expr *)e->data);
            my_free(e);
            break;
        case SEQUENCE_HOLDER:;
        {
            expr *curr = e;
            expr *res = NULL;
            while (curr) {
                free_tree((expr *)curr->data);
                expr *tmp = curr;
                curr = curr->next;
                my_free(tmp);
            }
            break;
        }
        case NUMBER:
            my_free(e);
            break;
        case SYMBOL:;
            symbol *sym = symbol_find((char *)e->data);
            if (sym != NULL) {
                my_free(sym);
            }
            /* The data field contains a string, the name of the symbol, so we need to
               my_free that as well */
            my_free((char *)e->data);
            my_free(e);
            break;
        case LIST:
            free_tree((expr *)e->data);
            my_free(e);
            break;
        case PROC_SYMBOL:;
            expr *first = e;
            expr *curr = first->next;
            my_free((char *)first->data);
            my_free(first);
            expr *res = NULL;
            while (curr) {
                expr *next = curr->next;
                free_tree(curr);
                curr = next;
            }
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
        case SEQUENCE_HOLDER:;
        {
            expr *curr = e;
            expr *res = NULL;
            while (curr) {
                res = eval((expr *)curr->data);
                curr = curr->next;
            }
            /* The result of the last eval is the final result */
            return res;
        }
        case NUMBER:
            return e;
        case SYMBOL:;
            symbol *sym = symbol_find((char *)e->data);
            if (sym != NULL) {
                return sym->e;
            }
            printf("WARNING: Found symbol with no value: %s. There is probably something wrong.\n",
                   (char *)e->data);
            return e;
        case LIST:
            return eval((expr *)e->data);
        case PROC_SYMBOL:;
            expr *proc = e;
            expr *arg = e->next;
            expr *res;

            /* If this is a defun, we should not evaluate the arguments */
            if (strcmp((char *)(proc->data), "defun") == 0) {
                defun(arg, &res);
                return res;
            }
            /* If this is a define, we should only evaluate one of the arguments */
            if (strcmp((char *)(proc->data), "define") == 0) {
                define(arg, &res);
                return res;
            }

            /* Evaluate the arguments, and build a list of those evaluated arguments */
            expr *prev_arg = NULL;
            expr *first_arg = NULL;
            while (arg) {
                expr *ev = eval(arg);

                expr *new = my_malloc(sizeof(expr));
                new->type = ev->type;
                new->data = ev->data;
                new->next = NULL;
                if (!first_arg)
                    first_arg = new;
                if (prev_arg)
                    prev_arg->next = new;
                prev_arg = new;

                expr *tmp = arg;
                arg = arg->next;
            }

            if (strcmp((char *)(proc->data), "+") == 0) {
                add(first_arg, &res);
            } else if (strcmp((char *)(proc->data), "-") == 0) {
                sub(first_arg, &res);
            } else if (strcmp((char *)(proc->data), "cons") == 0) {
                cons(first_arg, &res);
            } else if (strcmp((char *)(proc->data), "car") == 0) {
                car(first_arg, &res);
            } else if (strcmp((char *)(proc->data), "cdr") == 0) {
                cdr(first_arg, &res);
            } else {
                symbol *sym = symbol_find((char *)(proc->data));
                if (sym != NULL) {
                    if (sym->e->type == DEFUN) {
                        res = function_invocation(sym, first_arg);
                    } else if (sym->e->type == DEFINE) {
                        printf("ERROR: Cant use variable %s as a function\n", (char*)proc->data);
                        return NULL;
                    }
                } else {
                    printf("ERROR: No function identified for proc %s\n", (char*)(proc->data));
                }
            }

            expr *free_arg = first_arg;
            while (free_arg) {
                expr *tmp = free_arg;
                free_arg = free_arg->next;
                free_tree(tmp);
            }

            return res;
        default:
            printf("no action\n");
            return 0;
    }
}
