#ifndef EVAL_H_
#define EVAL_H_

#include <parser.h>
#include <symbol.h>

#include <user_stdio.h>
#include <stdint.h>


int quasiquote_eval(expr **arg);
int function_invocation(symbol *sym, expr *args, expr **out);
int eval(expr *e, expr **out);

#endif // EVAL_H_
