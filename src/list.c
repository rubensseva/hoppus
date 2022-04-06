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

/** Splice "spliced" right after the first entry in "list" */
int list_splice(expr *list, expr* spliced) {
    if (list->type != CONS || spliced->type != CONS) {
        printf("ERROR: LIST: When splicing lists, both arguments must be cons cells");
        return -1;
    }
    expr *old_cdr = list->cdr;
    list->cdr = spliced;
    expr *curr = spliced;
    while (!list_end(curr->cdr))
        curr = curr->cdr;
    curr->cdr = old_cdr;
    return 0;
}
