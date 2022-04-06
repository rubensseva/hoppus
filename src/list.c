#include "list.h"

#include "eval.h"

/**
   When iterating through the list, a NULL pointer signifies the end
   of the list */
int list_end(expr *e) {
    return e == NULL;
}

unsigned int list_length(expr *e) {
    expr *curr = e; unsigned int count = 0;
    while (!(list_end(curr))) {
        curr = curr->cdr;
        count++;
    }
    return count;
}

