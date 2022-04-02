#ifndef LIST_H_
#define LIST_H_

#include "eval.h"

#define for_each(curr)                                                  \
    for(int __for_each__i = 0, __for_each__size = list_length(curr);    \
        __for_each__i < __for_each__size;                               \
        curr = curr->cdr, __for_each__i++)

int list_end(expr *e);
unsigned int list_length(expr *e);

#endif // LIST_H_
