#ifndef MALLOC1_H_
#define MALLOC1_H_

/* Config for clisp, without microkernel */
// #define NULL (void *)0x0;
#define __USER_DATA
#define __USER_TEXT
// #define MALLOC_HEAP_SIZE 65536
// #define MALLOC_HEAP_SIZE 131072
#define MALLOC_HEAP_SIZE 262144

typedef struct mem_node_t {
    char *base;
    unsigned int size;
} mem_node;

typedef struct header header;
struct header {
    unsigned int size;
    header *next;
};

int gc_init();
int gc_maybe_mark_and_sweep();

void *gc_malloc(unsigned int size);
void gc_free(void *base);

#endif // MALLOC1_H_
