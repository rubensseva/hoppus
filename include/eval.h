#ifndef EVAL_H_
#define EVAL_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "parser.h"

expr *free_tree(expr *e);
expr *eval(expr *e);

#endif // EVAL_H_
