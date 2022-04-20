#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "parser.h"

int bi_defun(expr *arg, expr **out);
int bi_define(expr *arg, expr **out);
int bi_add(expr *arg, expr **out);
int bi_sub(expr *arg, expr **out);
int bi_mult(expr *arg, expr **out);
int bi_div(expr *arg, expr **out);
int bi_cons(expr *arg, expr **out);
int bi_car(expr *arg, expr **out);
int bi_cdr(expr *arg, expr **out);
int bi_progn(expr *arg, expr **out);
int bi_if(expr *arg, expr **out);
int bi_cond(expr *arg, expr **out);
int bi_print(expr *arg, expr **out);
int bi_equal(expr *arg, expr **out);
int bi_gt(expr *arg, expr **out);
int bi_lt(expr *arg, expr **out);
int bi_and(expr *arg, expr **out);
int bi_or(expr *arg, expr **out);
int bi_quote(expr *arg, expr **out);
int bi_defmacro(expr *arg, expr **out);
int bi_macroexpand(expr *arg, expr **out);
int bi_quasiquote(expr *arg, expr **out);
int bi_comma(expr *arg, expr **out);
int bi_comma_at(expr *arg, expr **out);

#endif // BUILTINS_H_
