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

char *get_malloc_heap();
header *get_malloc_used_list();
int mem_list_insert(mem_node mem_node);
int mem_list_insert_i(unsigned int index, mem_node mem_node);
int mem_list_remove_i(unsigned int index);
int mem_list_remove(char *base);
void *malloc1(unsigned int size);
void free1(void *base);

#endif // MALLOC1_H_
