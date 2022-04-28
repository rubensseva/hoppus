#ifndef GC_H_
#define GC_H_

#include <stdint.h>

/* Config for clisp, without microkernel */
// #define NULL (void *)0x0;
// #define MALLOC_HEAP_SIZE 65536
// #define MALLOC_HEAP_SIZE 131072
// #define MALLOC_HEAP_SIZE 262144
// #define MALLOC_HEAP_SIZE 524288
// #define MALLOC_HEAP_SIZE 2097152

/* Struct for used memory block. This will be embedded at the start of the block it is referring to.
   The size field is not including the header size.
   The LSB of the next pointer is used by gc to specify if the block is marked or not for gc. When
   using the next field, you probably need to untag it first */
typedef struct header header;
struct header {
    unsigned int size;
    header *next;
};

uint32_t align_up(uint32_t ptr, uint32_t size);
uint32_t align_down(uint32_t ptr, uint32_t size);

uint32_t gc_stats_get_num_malloc();
uint32_t gc_stats_get_allocated_total();

int gc_init();
int gc_maybe_mark_and_sweep();

void *gc_malloc(unsigned int size);

#endif // GC_H_
