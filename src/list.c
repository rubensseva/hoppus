#include "list.h"

#include "eval.h"

int list_end(expr *e) {
    return !(e && e->car);
}

unsigned int list_length(expr *e) {
    expr *curr = e; unsigned int count = 0;
    while (!(list_end(curr))) {
        curr = curr->cdr;
        count++;
    }
    return count;
}
