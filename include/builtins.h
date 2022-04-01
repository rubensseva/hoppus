#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "parser.h"

int bi_add(expr *arg, expr **res);
int bi_sub(expr *arg, expr **res);
int bi_cons(expr *arg, expr **res);
int bi_car(expr *arg, expr **res);
int bi_cdr(expr *arg, expr **res);
int bi_progn(expr *arg, expr **res);

#endif // BUILTINS_H_
