#ifndef EVAL_H_
#define EVAL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "parser.h"

expr *free_tree(expr *e);
int print_expr(expr *e);
int eval(expr *e, expr **res);

#endif // EVAL_H_
