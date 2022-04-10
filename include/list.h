#ifndef LIST_H_
#define LIST_H_

#include "eval.h"

/**
  Macro that provides a uniform way of iterating cons cells that form lists.

  Some usages of this utilize the "break" and "continue" keywords, so if this
  macro is changed to something other than a loop, it will probably introduce
  some bugs */
#define for_each(curr)                          \
    for(; !list_end(curr); curr = curr->cdr)

int list_end(expr *e);
unsigned int list_length(expr *e);

#endif // LIST_H_
