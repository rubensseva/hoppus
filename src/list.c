#include <list.h>
#include <eval.h>

#include <link.h>

/**
   When iterating through the list, a NULL pointer signifies the end
   of the list */
__USER_TEXT int list_end(expr *e) {
    return e == NULL;
}

__USER_TEXT unsigned int list_length(expr *e) {
    expr *curr = e; unsigned int count = 0;
    while (!(list_end(curr))) {
        curr = cdr(curr);
        count++;
    }
    return count;
}

