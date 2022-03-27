#ifndef SYMBOL_H_
#define SYMBOL_H_

#include "ir.h"

typedef struct symbol_t {
    char *name;
    expr val;
} symbol;

#endif // SYMBOL_H_
