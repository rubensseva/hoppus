#ifndef EVAL_H_
#define EVAL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "parser.h"
#include "symbol.h"

int quasiquote_eval(expr **arg);
int function_invocation(symbol *sym, expr *args, expr **out);
int eval(expr *e, expr **out);

#endif // EVAL_H_
