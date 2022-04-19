#include <stdint.h>
#include <stdio.h>
#include "lib/malloc1.h"
#include "utility.h"

#define UNTAG(p) ((header *)(((uint64_t) (p)) & 0xFFFFFFFFFFFFFFFC))

/* provided by linker, sdata is from customized default linker script,
   not in regular default linker script */
extern char end, sdata;

header *used = NULL;
header *prev_malloced = NULL;

uint64_t stack_start;

char malloc_heap[MALLOC_HEAP_SIZE];
uint64_t gc_allocated_size = 0;

header *heap_start = NULL;
header *heap_end = NULL;

unsigned int gc_get_cap() {
    return MALLOC_HEAP_SIZE;
}
unsigned int gc_calc_allocated() {
    unsigned int count = 0;
    for (header *u = used; u != NULL; u = UNTAG(u->next)) {
        count += u->size + 1;
    }
    return count * sizeof(header);
}


uint64_t align_up(uint64_t ptr) {
    if (ptr % sizeof(header) != 0)
        ptr += sizeof(header) - ptr % sizeof(header);
    return ptr;
}
uint64_t align_down(uint64_t ptr) {
    if (ptr % sizeof(header) != 0)
        ptr -= ptr % sizeof(header);
    return ptr;
}

header *new_next_ptr(header *old_next, header *new_next) {
    return (header *)((uint64_t)UNTAG(new_next) | ((uint64_t)old_next & 1));
}

/* Inspired from https://maplant.com/gc.html */
int gc_init() {
    static int initted;
    FILE *statfp;

    if (initted)
        return 0;

    initted = 1;

    statfp = fopen("/proc/self/stat", "r");
    if (statfp == NULL) {
        perror("open");
        printf("ERROR: GC: GC_INIT: Opening stat\n");
        return -1;
    }
    fscanf(statfp,
           "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
           "%*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld "
           "%*ld %*ld %*ld %*ld %*llu %*lu %*ld "
           "%*lu %*lu %*lu %lu", &stack_start);
    fclose(statfp);

    heap_start = (header *)align_up((uint64_t)malloc_heap);
    heap_end = (header *)align_down((uint64_t)(malloc_heap + MALLOC_HEAP_SIZE));

    gc_allocated_size = 0;
    used = NULL;
    return 0;
}


void gc_scan_follow_obj(header *u) {
    for (header *p = u + 1; p <= (header *)((uint64_t *)(u + 1 + u->size) - 1); p = (header *)((uint64_t *)p + 1)) {
        uint64_t v = *(uint64_t *)p;
        if (v < (uint64_t)malloc_heap || v >= (uint64_t)(malloc_heap + MALLOC_HEAP_SIZE)) {
            continue;
        }
        /* For each memory region, check if it points to any other entry in the used list */
        for (header *uu = used; uu != NULL; uu = UNTAG(uu->next)) {
            if (((uint64_t)uu->next & 1) == 1)
                continue;

            if (v >= (uint64_t)(uu + 1) && v <= (uint64_t)(uu + 1 + uu->size)) {
                /* Mark as live */
                uu->next = (header *)((uint64_t)uu->next | 1);
                gc_scan_follow_obj(uu);
            }
        }
    }
}

int gc_scan_heap() {
    header *heap = (header *) align_up((uint64_t)malloc_heap);
    header *heap_end = (header *) ((char *)heap + MALLOC_HEAP_SIZE);

    uint64_t *b = (uint64_t *)heap;

    /* For each entry in used list */
    for(header *u = used; u != NULL; u = UNTAG(u->next)) {
        /* If the block is not marked, skip it */
        if (((uint64_t)u->next & 1) == 0)
            continue;

        /* Go through all the memory for that used list entry */
        for (header *p = u + 1; p <= (header *)((uint64_t *)(u + 1 + u->size) - 1); p = (header *)((uint64_t *)p + 1)) {
            uint64_t v = *(uint64_t *)p;
            if (v < (uint64_t)malloc_heap || v >= (uint64_t)(malloc_heap + MALLOC_HEAP_SIZE)) {
                continue;
            }
            /* For each memory region, check if it points to any other entry in the used list */
            for (header *uu = used; uu != NULL; uu = UNTAG(uu->next)) {
                if (((uint64_t)uu->next & 1) == 1)
                    continue;

                if (v >= (uint64_t)(uu + 1) && v <= (uint64_t)(uu + 1 + uu->size)) {
                    /* Mark as live */
                    uu->next = (header *)((uint64_t)uu->next | 1);
                    gc_scan_follow_obj(uu);
                }
            }
        }
    }
    return 0;
}

int gc_scan_region(uint64_t *start, uint64_t *end) {
    for (uint64_t *p = start; p <= end - 1; p += 1) {
        uint64_t v = *p;
        if (v < (uint64_t)malloc_heap || v >= (uint64_t)(malloc_heap + MALLOC_HEAP_SIZE)) {
            continue;
        }
        if (p == (uint64_t*)used || p == (uint64_t*)prev_malloced) {
            continue;
        }
        /* For each memory region, check if it points to any entry in the used list */
        for (header *u = used; u != NULL; u = UNTAG(u->next)) {
            if (((uint64_t)u->next & 1) == 1) {
                continue;
            }
            if (v >= (uint64_t)(u + 1) && v <= (uint64_t)(u + 1 + u->size)) {
                /* Mark as live */
                u->next = (header *)((uint64_t)u->next | 1);
            }
        }
    }
    return 0;
}

int gc_sweep() {
    unsigned int num_sweeped = 0;
    // for (header *u = used, *prev = NULL; u != NULL; prev = u, u = UNTAG(u->next)) {
    header *u = used, *prev = NULL;
    while (u != NULL) {
        if (((uint64_t)u->next & 1) == 0) {
            num_sweeped++;
            gc_allocated_size -= (u->size + 1) * sizeof(header);
            if (u == prev_malloced)
                prev_malloced = prev;
            if (u == used) {
                used = UNTAG(u->next);
            } else {
                prev->next = new_next_ptr(prev->next, u->next);
                u = UNTAG(prev->next);
                continue;
            }
        }
        prev = u;
        u = UNTAG(u->next);
    }
    printf("INFO: GC: sweeped %d objects\n", num_sweeped);
    return 0;
}

int gc_dump_info() {
    int count = 0, marked = 0, unmarked = 0;
    for (header *u = used; u != NULL; u = UNTAG(u->next)) {
        count++;
        if (((uint64_t)u->next & 1) == 1) {
            marked++;
        } else {
            unmarked++;
        }
    }
    printf("INFO: GC: %d allocated objects\n", count);
    printf("INFO: GC: %d marked objects\n", marked);
    printf("INFO: GC: %d unmarked objects\n", unmarked);
    return 0;
}

int gc_mark_and_sweep() {
    uint64_t *stack_end;
    register void *sp asm ("sp");
    stack_end = sp;

    printf("-----------------------------\n");
    printf("INFO: GC: starting\n");

    /* Reset gc marks */
    for (header *u = used; u != NULL; u = UNTAG(u->next))
        u->next = UNTAG(u->next);

    if (used == NULL) {
        return 0;
    }
    /* Scan data segments. Skip the heap. Since "end" is the last
       address PAST bss, we need to subtract 1 */
    gc_scan_region((uint64_t *)&sdata, (uint64_t *)malloc_heap);
    gc_scan_region((uint64_t *)(malloc_heap + MALLOC_HEAP_SIZE), (uint64_t *)&end - 1);

    gc_scan_region(stack_end, (uint64_t *)stack_start);
    gc_scan_heap();


    printf("INFO: GC: scan finished\n");
    gc_dump_info();
    gc_sweep();
    printf("-----------------------------\n");

    return 0;
}

int gc_maybe_mark_and_sweep() {
    if (gc_allocated_size >= MALLOC_HEAP_SIZE >> 1) {
        printf("INFO: GC: running gc, alloc: %lu / %d\n", gc_allocated_size, MALLOC_HEAP_SIZE);
        return gc_mark_and_sweep();
    }
    return 0;
}


void gc_alloc_dump() {
    printf("INFO: GC: heap start %p, end %p\n", malloc_heap, malloc_heap + MALLOC_HEAP_SIZE);
    int i = 0;
    for (header *u = used, *tag_addr = used->next;
         u != NULL && UNTAG(u->next) != NULL;
         u = UNTAG(u->next), tag_addr = u->next) {
        printf("INFO: GC: %d: tag_addr: %p, %p -> %p, size: %lu\n", i++, tag_addr, u, u + u->size, u->size * sizeof(header));
    }
}

void *gc_malloc(unsigned int size) {
    if (size <= 0) {
        printf("ERROR: GC: MALLOC: got request to malloc with size 0\n");
        return (void *)NULL;
    }

    if (!heap_start || !heap_end) {
        printf("ERROR: GC: MALLOC: missing heap start or end\n");
        return (void *)NULL;
    }

    unsigned int required_units = (align_up(size) + sizeof(header)) / sizeof(header);
    if (required_units * sizeof(header) > MALLOC_HEAP_SIZE) {
        printf("ERROR: GC: MALLOC: got request to allocate %d bytes, which means I need to allocate a total of %lu, but its more than the size of the heap which is %d\n",
               size,
               required_units * sizeof(header),
               MALLOC_HEAP_SIZE);
        return (void *)NULL;
    }

    /* Set up used list if it is empty */
    if (used == NULL) {
        header *free_base = (header *)heap_start;
        free_base->size = required_units - 1;
        free_base->next = NULL;
        used = free_base;
        prev_malloced = free_base;
        gc_allocated_size += (free_base->size + 1) * sizeof(header);
        return free_base + 1;
    }

    if (prev_malloced == NULL) {
        prev_malloced = used;
    }
    int first_round = 1;
    for (header *u = UNTAG(prev_malloced);; u = UNTAG(u->next)) {
        if (!first_round && u == prev_malloced)
            break;
        first_round = 0;
        if (u == NULL)
            u = used;

        header *free_base, *u_end = u + 1 + u->size;

        /* Space between this block and the next block. Most common case, so lets check for it first. */
        if (u->next != NULL && required_units < UNTAG(u->next) - u_end) {
            free_base = u_end;
            free_base->next = u->next;
            u->next = new_next_ptr(u->next, free_base); // TODO: Why do we not use new_next_ptr here?
        /* Start of the used list, and space before first block */
        } else if (u == used && required_units < u - (header *)heap_start) {
            free_base = (header *)heap_start;
            free_base->next = u;
            used = free_base;
        /* End of the list, and space after the last block */
        } else if (UNTAG(u->next) == NULL && required_units < (header *)heap_end - u_end) {
            free_base = u_end;
            free_base->next = NULL;
            u->next = new_next_ptr(u->next, free_base);
        /* If none of the above clauses are true, go to the next block */
        } else {
            continue;
        }
        free_base->size = required_units - 1;
        prev_malloced = free_base;
        gc_allocated_size += (free_base->size + 1) * sizeof(header);
        return free_base + 1;
    }

    printf("ERROR: GC: MALLOC: couldnt find space for size %d, units %d\n", size, required_units);
    printf("INFO: GC: MALLOC: currently allocated %d / %d\n", gc_calc_allocated(), gc_get_cap());
    return (void *)NULL;
}

/* TODO: Shouldnt really need this once gc is working */
void gc_free(void *ptr) {
    if (ptr == 0x0) {
        printf("ERROR: GC: FREE: Attempt to free NULL\n");
        return;
    }
    for (header *u = used, *prev = NULL; u != NULL; prev = u, u = UNTAG(u->next)) {
        if (u + 1 == (header *) ptr) {
            gc_allocated_size -= (u->size + 1) * sizeof(header);
            if (prev) {
                prev->next = new_next_ptr(prev->next, u->next);
            } else {
                used = UNTAG(u->next);
            }
            if (u == prev_malloced)
                prev_malloced = prev;
            break;
        }
    }
}
