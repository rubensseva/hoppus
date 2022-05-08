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

uintptr_t align_up(uintptr_t ptr, uint32_t size);
uintptr_t align_down(uintptr_t ptr, uint32_t size);

uint32_t gc_stats_get_num_malloc();
uint32_t gc_stats_get_allocated_total();

int gc_init();
int gc_maybe_mark_and_sweep();

void *gc_malloc(unsigned int size);

/* The size should be that of a cons cell, redeclaring that size here to avoid importing
 too much LISP related stuff.*/
struct cons_cell_size {
    uintptr_t foo;
    uintptr_t bar;
};
typedef struct cons_cell_size small_obj;

#ifdef HOPPUS_X86
#define HEADER_NEXT_PTR_MASK 0xFFFFFFFFFFFFFFFC
#endif
#ifdef HOPPUS_RISCV_F9
#define HEADER_NEXT_PTR_MASK 0xFFFFFFFC
#endif

#define UNTAG(p) ((header *)(((uintptr_t) (p)) & HEADER_NEXT_PTR_MASK))

#endif // GC_H_
