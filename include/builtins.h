#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "parser.h"

int bi_defun(expr *arg, expr **res);
int bi_define(expr *arg, expr **res);
int bi_add(expr *arg, expr **res);
int bi_sub(expr *arg, expr **res);
int bi_cons(expr *arg, expr **res);
int bi_car(expr *arg, expr **res);
int bi_cdr(expr *arg, expr **res);
int bi_progn(expr *arg, expr **res);
int bi_if(expr *arg, expr **res);
int bi_print(expr *arg, expr **res);
int bi_equal(expr *arg, expr **res);
int bi_gt(expr *arg, expr **res);
int bi_lt(expr *arg, expr **res);
int bi_and(expr *arg, expr **res);
int bi_or(expr *arg, expr **res);

#endif // BUILTINS_H_
