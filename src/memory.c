#include <stdlib.h>

#include "memory.h"
#include "lib/malloc1.h"

/* Some simple inderection functions for easialy changing malloc implementations
   between custom malloc functions and std lib */

void *my_malloc(unsigned int size) {
    return gc_malloc(size);
    /* return malloc(size); */
}
void my_free(void *ptr) {
    gc_free(ptr);
    /* free(ptr); */
}
