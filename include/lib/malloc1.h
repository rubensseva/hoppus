#ifndef MALLOC1_H_
#define MALLOC1_H_

typedef struct mem_node_t {
    char *base;
    unsigned int size;
} mem_node;

int mem_list_insert(mem_node mem_node);
int mem_list_insert_i(unsigned int index, mem_node mem_node);
int mem_list_remove_i(unsigned int index);
int mem_list_remove(char *base);
void *malloc1(unsigned int size);
void free1(void *base);

#endif // MALLOC1_H_
