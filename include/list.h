#ifndef LIST_H_
#define LIST_H_

#include "eval.h"

/**
  Macro that provides a uniform way of iterating cons cells that form lists.

  Some usages of this utilize the "break" and "continue" keywords, so if this
  macro is changed to something other than a loop, it will probably introduce
  some bugs */
#define for_each(curr)                                                  \
    for(int __for_each__i = 0, __for_each__size = list_length(curr);    \
        __for_each__i < __for_each__size;                               \
        curr = curr->cdr, __for_each__i++)

int list_end(expr *e);
unsigned int list_length(expr *e);
int list_splice(expr *list, expr* spliced);

#endif // LIST_H_
