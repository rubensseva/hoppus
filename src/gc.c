#include <stdint.h>

#include <USER_stdio.h>
#include <gc.h>
#include <clisp_utility.h>
#include <link.h>

#define UNTAG(p) ((header *)(((uint32_t) (p)) & 0xFFFFFFFC))

/* provided by linker, sdata is from customized default linker script,
   not in regular default linker script */
extern char end, sdata;

__USER_DATA header *used = NULL;
__USER_DATA header *prev_malloced = NULL;

// char malloc_heap[MALLOC_HEAP_SIZE];
__USER_DATA uint32_t gc_allocated_size = 0;

__USER_DATA header *heap_start = NULL;
__USER_DATA header *heap_end = NULL;
__USER_DATA uint32_t heap_size = 0;


__USER_DATA uint32_t gc_stats_num_malloc = 0;
__USER_DATA uint32_t gc_stats_allocated_total = 0;

__USER_TEXT uint32_t gc_stats_get_num_malloc() {
    return gc_stats_num_malloc;
}
__USER_TEXT uint32_t gc_stats_get_allocated_total() {
    return gc_stats_allocated_total;
}

__USER_TEXT unsigned int gc_get_cap() {
    return heap_size;
}
__USER_TEXT unsigned int gc_calc_allocated() {
    unsigned int count = 0;
    for (header *u = used; u != NULL; u = UNTAG(u->next)) {
        count += u->size + 1;
    }
    return count * sizeof(header);
}


__USER_TEXT uint32_t align_up(uint32_t ptr) {
    if (ptr % sizeof(header) != 0)
        ptr += sizeof(header) - ptr % sizeof(header);
    return ptr;
}
__USER_TEXT uint32_t align_down(uint32_t ptr) {
    if (ptr % sizeof(header) != 0)
        ptr -= ptr % sizeof(header);
    return ptr;
}

__USER_TEXT header *new_next_ptr(header *old_next, header *new_next) {
    return (header *)((uint32_t)UNTAG(new_next) | ((uint32_t)old_next & 1));
}

/* Inspired from https://maplant.com/gc.html */
__USER_TEXT int gc_init() {
    static int initted;
    /* FILE *statfp;

    if (initted)
        return 0;

    initted = 1;

    /* statfp = fopen("/proc/self/stat", "r"); */
    /* if (statfp == NULL) { */
    /*     perror("open"); */
    /*     user_puts("ERROR: GC: GC_INIT: Opening stat\n"); */
    /*     return -1; */
    /* } */
    /* fscanf(statfp, */
    /*        "%*d %*s %*c %*d %*d %*d %*d %*d %*u " */
    /*        "%*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld " */
    /*        "%*ld %*ld %*ld %*ld %*llu %*lu %*ld " */
    /*        "%*lu %*lu %*lu %lu", &stack_start); */
    /* fclose(statfp); */

    heap_start = (header *)align_up((uint32_t)&user_thread_heap_start);
    heap_end = (header *)align_down((uint32_t)&user_thread_heap_end);
    heap_size = (char *)heap_end - (char *)heap_start;

    /* heap_start = (header *)align_up((uint32_t)malloc_heap); */
    /* heap_end = (header *)align_down((uint32_t)(malloc_heap + MALLOC_HEAP_SIZE)); */

    gc_allocated_size = 0;
    used = NULL;
    return 0;
}


__USER_TEXT int gc_is_skippable_ptr(uint32_t *p) {
    return (*p < (uint32_t)heap_start || *p >= (uint32_t)heap_end) || /* If points to outside heap */
        (p == (uint32_t*)used || p == (uint32_t*)prev_malloced); /* If we are checking a gc address */
}

__USER_TEXT void gc_scan_follow_obj(header *u) {
    for (header *p = u + 1; p <= (header *)((uint32_t *)(u + 1 + u->size) - 1); p = (header *)((uint32_t *)p + 1)) {
        if (gc_is_skippable_ptr((uint32_t *) p))
            continue;
        uint32_t v = *(uint32_t *)p;
        /* For each memory region, check if it points to any other entry in the used list */
        for (header *uu = used; uu != NULL; uu = UNTAG(uu->next)) {
            if (((uint32_t)uu->next & 1) == 1)
                continue;

            if (v >= (uint32_t)(uu + 1) && v <= (uint32_t)(uu + 1 + uu->size)) {
                /* Mark as live */
                uu->next = (header *)((uint32_t)uu->next | 1);
                gc_scan_follow_obj(uu);
            }
        }
    }
}

__USER_TEXT int gc_scan_heap() {
    uint32_t *b = heap_start;

    for(header *u = used; u != NULL; u = UNTAG(u->next)) {
        /* If the block is not marked, skip it */
        if (((uint32_t)u->next & 1) == 0)
            continue;
        /* Go through the memory of the used block */
        for (header *p = u + 1; p <= (header *)((uint32_t *)(u + 1 + u->size) - 1); p = (header *)((uint32_t *)p + 1)) {
            if (gc_is_skippable_ptr((uint32_t *) p))
                continue;
            uint32_t v = *(uint32_t *)p;
            for (header *uu = used; uu != NULL; uu = UNTAG(uu->next)) {
                if (((uint32_t)uu->next & 1) == 1)
                    continue;

                if (v >= (uint32_t)(uu + 1) && v <= (uint32_t)(uu + 1 + uu->size)) {
                    /* Mark as live */
                    uu->next = (header *)((uint32_t)uu->next | 1);
                    gc_scan_follow_obj(uu);
                }
            }
        }
    }
    return 0;
}

__USER_TEXT int gc_scan_region(uint32_t *start, uint32_t *end) {
    user_printf("INFO: GC: scanning region from %x to %x\n", start, end);
    for (uint32_t *p = start; p <= end - 1; p += 1) {
        if (gc_is_skippable_ptr(p))
            continue;
        uint32_t v = *p;
        for (header *u = used; u != NULL; u = UNTAG(u->next)) {
            /* If the block is marked, skip it */
            if (((uint32_t)u->next & 1) == 1) {
                continue;
            }
            /* If an address in the region points to a used block, mark it as live */
            if (v >= (uint32_t)(u + 1) && v <= (uint32_t)(u + 1 + u->size)) {
                u->next = (header *)((uint32_t)u->next | 1);
            }
        }
    }
    return 0;
}

__USER_TEXT int gc_sweep() {
    unsigned int num_sweeped = 0;
    // for (header *u = used, *prev = NULL; u != NULL; prev = u, u = UNTAG(u->next)) {
    header *u = used, *prev = NULL;
    while (u != NULL) {
        if (((uint32_t)u->next & 1) == 0) {
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
    user_printf("INFO: GC: sweeped %d objects\n", num_sweeped);
    return 0;
}

__USER_TEXT int gc_dump_info() {
    int count = 0, marked = 0, unmarked = 0;
    for (header *u = used; u != NULL; u = UNTAG(u->next)) {
        count++;
        if (((uint32_t)u->next & 1) == 1) {
            marked++;
        } else {
            unmarked++;
        }
    }
    user_printf("INFO: GC: %d allocated objects\n", count);
    user_printf("INFO: GC: %d marked objects\n", marked);
    user_printf("INFO: GC: %d unmarked objects\n", unmarked);
    return 0;
}

__USER_TEXT int gc_mark_and_sweep() {
    uint32_t *sp;
    register void *_sp asm ("sp");
    sp = _sp;

    user_puts("-----------------------------\n");
    user_puts("INFO: GC: starting\n");

    /* Reset gc marks */
    for (header *u = used; u != NULL; u = UNTAG(u->next))
        u->next = UNTAG(u->next);

    if (used == NULL) {
        return 0;
    }
    /* Scan data segments. Skip the heap. Since "end" is the last
       address PAST bss, we need to subtract 1 */
    /* gc_scan_region((uint32_t *)&sdata, (uint32_t *)malloc_heap); */
    /* gc_scan_region((uint32_t *)(malloc_heap + MALLOC_HEAP_SIZE), (uint32_t *)&end - 1); */

    gc_scan_region((uint32_t *) &user_bss_start, (uint32_t *) &user_bss_end);
    gc_scan_region((uint32_t *) &user_data_misc_start, (uint32_t *) &user_data_misc_end);

    gc_scan_region(sp, (uint32_t *)&user_thread_stack_end);
    gc_scan_heap();


    user_puts("INFO: GC: scan finished\n");
    gc_dump_info();
    user_printf("INFO: GC: %d / %d bytes allocated\n", gc_allocated_size, heap_size);
    user_printf("INFO: GC: %d / %d recalculated\n", gc_calc_allocated(), heap_size);
    gc_sweep();
    user_printf("INFO: GC: %d num mallocs\n", gc_stats_get_num_malloc());
    user_printf("INFO: GC: %d total alloc\n", gc_stats_get_allocated_total());
    user_printf("INFO: GC: %d / %d bytes allocated\n", gc_allocated_size, heap_size);
    user_printf("INFO: GC: %d / %d recalculated\n", gc_calc_allocated(), heap_size);
    user_puts("-----------------------------\n");

    return 0;
}

/* __USER_TEXT int gc_maybe_mark_and_sweep() { */
/*     if (gc_allocated_size >= (MALLOC_HEAP_SIZE >> 1)) { */
/*         user_printf("INFO: GC: gc running %lu / %d\n", gc_allocated_size, MALLOC_HEAP_SIZE); */
/*         user_printf("INFO: GC: gc recalculated %u / %d\n", gc_calc_allocated(), MALLOC_HEAP_SIZE); */
/*         int ret_code = gc_mark_and_sweep(); */
/*         user_printf("INFO: GC: gc done %lu / %d\n", gc_allocated_size, MALLOC_HEAP_SIZE); */
/*         user_printf("INFO: GC: gc recalculated  %u / %d\n", gc_calc_allocated(), MALLOC_HEAP_SIZE); */
/*         return ret_code; */
/*     } */
/*     return 0; */
/* } */


__USER_TEXT void gc_alloc_dump() {
    user_printf("INFO: GC: heap start %p, end %p\n", heap_start, heap_end);
    int i = 0;
    for (header *u = used, *tag_addr = used->next;
         u != NULL && UNTAG(u->next) != NULL;
         u = UNTAG(u->next), tag_addr = u->next) {
        user_printf("INFO: GC: %d: tag_addr: %p, %p -> %p, size: %lu\n", i++, tag_addr, u, u + u->size, u->size * sizeof(header));
    }
}

__USER_TEXT void *gc_malloc_units(unsigned int required_units) {
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
        if (UNTAG(u->next) != NULL && required_units < UNTAG(u->next) - u_end) {
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
    return NULL;
}

__USER_TEXT void *gc_malloc(unsigned int size) {
    if (size <= 0) {
        user_puts("ERROR: GC: MALLOC: got request to malloc with size 0\n");
        return (void *)NULL;
    }
    if (!heap_start || !heap_end) {
        user_puts("ERROR: GC: MALLOC: missing heap start or end\n");
        return (void *)NULL;
    }

    unsigned int required_units = (align_up(size) + sizeof(header)) / sizeof(header);
    if (required_units * sizeof(header) > heap_size) {
        user_printf("ERROR: GC: MALLOC: got request to allocate %d bytes, which means I need to allocate a total of %d, but its more than the size of the heap which is %d\n",
               size,
               required_units * sizeof(header),
               heap_size);
        return (void *)NULL;
    }

    void *ptr = gc_malloc_units(required_units);
    if (ptr == NULL) {
        /* no space, run gc, then try again */
        gc_mark_and_sweep();
        ptr = gc_malloc_units(required_units);
    }

    if (ptr == NULL) {
        user_printf("ERROR: GC: MALLOC: couldnt find space for size %d, units %d\n", size, required_units);
    }
    gc_stats_num_malloc += 1;
    gc_stats_allocated_total += required_units * sizeof(header);
    /* user_printf("INFO: GC: MALLOC: currently allocated %d / %d\n", gc_calc_allocated(), gc_get_cap()); */
    return ptr;
}
