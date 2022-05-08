#include <hoppus_memory.h>
#include <gc.h>
#include <hoppus_link.h>

/* Some simple inderection functions for easialy changing malloc implementations
   between custom malloc functions and std lib */

__USER_TEXT void *my_malloc(unsigned int size) {
    return gc_malloc(size);
    /* return malloc(size); */
}
