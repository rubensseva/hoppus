#include <gc.h>
#include <hoppus_utility.h>
#include <hoppus_stdio.h>
#include <hoppus_link.h>

#include <stdint.h>

#define UNTAG(p) ((header *)(((uint32_t) (p)) & 0xFFFFFFFC))

typedef uint64_t small_obj;

#ifdef HOPPUS_PLATFORM
#if HOPPUS_PLATFORM == HOPPUS_X86
#include <stdio.h>
#define MALLOC_HEAP_SIZE 4096
char malloc_heap[MALLOC_HEAP_SIZE];
uint64_t stack_end;
#elif HOPPUS_PLATFORM == HOPPUS_X86
uint32_t stack_end = &user_thread_stack_end;
#endif
#endif

/* 128 bytes for bitmap gives 1024 objects, which is 8192 bytes */
#define BITMAP_SIZE 128

/* provided by linker, sdata is from customized default linker script,
   not in regular default linker script */
extern char end, sdata;

__USER_DATA header *large_used = NULL;
__USER_DATA header *large_prev_malloced = NULL;

__USER_DATA int small_prev_malloced_byte = 0;

// char malloc_heap[MALLOC_HEAP_SIZE];
__USER_DATA uint32_t gc_allocated_size = 0;

__USER_DATA header *heap_start = NULL;
__USER_DATA header *heap_end = NULL;
__USER_DATA uint32_t heap_size = 0;

__USER_DATA header *heap_split = NULL;


__USER_DATA uint32_t gc_stats_num_malloc = 0;

/* 8 * 1024 = 8192 bits */
__USER_DATA uint8_t alloc_bitmap[BITMAP_SIZE];
__USER_DATA uint8_t marked_bitmap[BITMAP_SIZE];

__USER_TEXT int gc_bitmap_get(uint8_t bitmap[], int i) {
    int byte = i / 8;
    int bit_in_byte = (i % 8);
    int bit = (bitmap[byte] >> bit_in_byte) & 1;
    return bit;
}

__USER_TEXT void gc_bitmap_set(uint8_t bitmap[], int i, int val) {
    int byte = i / 8;
    int bit_in_byte = (i % 8);
    if (val) {
        bitmap[byte] |= 1 << bit_in_byte;
    } else {
        bitmap[byte] &= (~(1 << bit_in_byte));
    }
}

__USER_TEXT int gc_bitmap_addr_to_i(small_obj *o) {
    return (align_down((uint32_t)o, sizeof(small_obj)) - (uint32_t)heap_start) / sizeof(small_obj);
}

__USER_TEXT void gc_bitmap_init(uint8_t bitmap[]) {
    for (int i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0;
    }
}

__USER_TEXT void gc_bitmap_print(uint8_t bitmap[]) {
    for (int i = 0; i < BITMAP_SIZE * 8; i++) {
        hoppus_printf("%d", gc_bitmap_get(bitmap, i))
        if (i % 100 == 0)
            hoppus_puts("\n");
    }
    hoppus_puts("\n");
}

__USER_TEXT uint32_t gc_stats_get_num_malloc() {
    return gc_stats_num_malloc;
}

__USER_TEXT unsigned int gc_get_cap() {
    return heap_size;
}
__USER_TEXT unsigned int gc_calc_allocated() {
    unsigned int count = 0;
    for (header *u = large_used; u != NULL; u = UNTAG(u->next)) {
        count += u->size + 1;
    }
    return count * sizeof(header);
}


__USER_TEXT uint32_t align_up(uint32_t ptr, uint32_t size) {
    if (ptr % size != 0)
        ptr += size - ptr % size;
    return ptr;
}
__USER_TEXT uint32_t align_down(uint32_t ptr, uint32_t size) {
    if (ptr % size != 0)
        ptr -= ptr % size;
    return ptr;
}

__USER_TEXT header *new_next_ptr(header *old_next, header *new_next) {
    return (header *)((uint32_t)UNTAG(new_next) | ((uint32_t)old_next & 1));
}

__USER_TEXT int gc_init() {

#ifdef HOPPUS_PLATFORM
#if HOPPUS_PLATFORM == HOPPUS_X86
    FILE *statfp = fopen("/proc/self/stat", "r");
    if (statfp == NULL) {
        perror("open");
        printf("ERROR: GC: GC_INIT: Opening stat\n");
        return -1;
    }
    fscanf(statfp,
           "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
           "%*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld "
           "%*ld %*ld %*ld %*ld %*llu %*lu %*ld "
           "%*lu %*lu %*lu %lu", &stack_end);
    fclose(statfp);


    heap_start = (header *)align_up((uint32_t)malloc_heap, sizeof(header));
    heap_end = (header *)align_down((uint32_t)((char *) malloc_heap + MALLOC_HEAP_SIZE), sizeof(header));
#elif HOPPUS_PLATFORM == HOPPUS_RISCV_F9
    heap_start = (header *)align_up((uint32_t)&user_thread_heap_start, sizeof(header));
    heap_end = (header *)align_down((uint32_t)&user_thread_heap_end, sizeof(header));
#endif
#endif

    heap_size = (char *)heap_end - (char *)heap_start;

    heap_split = (header *)((small_obj *)heap_start + (BITMAP_SIZE * 8));

    gc_bitmap_init(alloc_bitmap);
    gc_bitmap_init(marked_bitmap);

    gc_allocated_size = 0;
    large_used = NULL;
    return 0;
}

__USER_TEXT int gc_is_skippable_ptr(uint32_t *p) {
    return (*p < (uint32_t)heap_start || *p >= (uint32_t)heap_end) || /* If points to outside heap */
        (p == (uint32_t*)large_used || p == (uint32_t*)large_prev_malloced); /* If we are checking a gc address */
}

__USER_TEXT void gc_scan_follow_region(uint32_t *start, uint32_t *end) {
    for (uint32_t *p = start; p <= end - 1; p++) {

        if (gc_is_skippable_ptr((uint32_t *) p))
            continue;
        uint32_t v = *(uint32_t *)p;

        /* If its in the small objects heap */
        if (v < (uint32_t)heap_split) {
            int i = gc_bitmap_addr_to_i((small_obj *)v);
            if (gc_bitmap_get(alloc_bitmap, i) && !gc_bitmap_get(marked_bitmap, i)) {
                gc_bitmap_set(marked_bitmap, i, 1);
                gc_scan_follow_region((uint32_t *)v, (uint32_t *)((small_obj *)v + 1));
            }
            continue;
        }

        /* If its in the large objects heap */
        for (header *uu = large_used; uu != NULL; uu = UNTAG(uu->next)) {
            if (v < (uint32_t)(uu))
                break;
            if (v >= (uint32_t)(uu + 1) && v <= (uint32_t)(uu + 1 + uu->size)) {
                if (((uint32_t)uu->next & 1) == 1)
                    break;
                uu->next = (header *)((uint32_t)uu->next | 1);
                gc_scan_follow_region((uint32_t *)(uu + 1), (uint32_t *)(uu + 1 + uu->size));
                break;
            }
        }
    }
}

__USER_TEXT int gc_scan_heap() {
    /* Scan small objects heap */
    for(int i = 0; i < BITMAP_SIZE * 8; i++) {
        /* If the block is not marked, skip it */
        if (!gc_bitmap_get(alloc_bitmap, i) || !gc_bitmap_get(marked_bitmap, i))
            continue;
        small_obj *o = &((small_obj *)heap_start)[i];
        gc_scan_follow_region((uint32_t *)o, (uint32_t *)(o + 1));
    }

    /* Scan large objects heap */
    for(header *u = large_used; u != NULL; u = UNTAG(u->next)) {
        /* If the block is not marked, skip it */
        if (((uint32_t)u->next & 1) == 0)
            continue;
        gc_scan_follow_region((uint32_t *)(u + 1), (uint32_t *)(u + 1 + u->size));
    }
    return 0;
}

__USER_TEXT int gc_scan_region(uint32_t *start, uint32_t *end) {
    hoppus_printf("INFO: GC: scanning region from %x to %x...\n", start, end);
    for (uint32_t *p = start; p <= end - 1; p += 1) {
        if (gc_is_skippable_ptr(p))
            continue;
        uint32_t v = *p;

        /* If its in the small objects heap */
        if (v < (uint32_t)heap_split) {
            int i = gc_bitmap_addr_to_i((small_obj *)v);
            if (gc_bitmap_get(alloc_bitmap, i) && !gc_bitmap_get(marked_bitmap, i)) {
                gc_bitmap_set(marked_bitmap, i, 1);
            }
            continue;
        }

        /* If its in the large objects heap */
        for (header *u = large_used; u != NULL; u = UNTAG(u->next)) {
            if (v < (uint32_t)(u))
                break;
            /* If an address in the region points to a used block, mark it as live */
            if (v >= (uint32_t)(u + 1) && v <= (uint32_t)(u + 1 + u->size)) {
                u->next = (header *)((uint32_t)u->next | 1);
                break;
            }
        }
    }
    return 0;
}

__USER_TEXT int gc_sweep() {
    unsigned int num_sweeped = 0;
    header *u = large_used, *prev = NULL;
    while (u != NULL) {
        if (((uint32_t)u->next & 1) == 0) {
            num_sweeped++;
            gc_allocated_size -= (u->size + 1) * sizeof(header);
            if (u == large_prev_malloced)
                large_prev_malloced = prev;
            if (u == large_used) {
                large_used = UNTAG(u->next);
            } else {
                prev->next = new_next_ptr(prev->next, u->next);
                u = UNTAG(prev->next);
                continue;
            }
        }
        prev = u;
        u = UNTAG(u->next);
    }

    unsigned int num_small_sweeped = 0;
    for (int i = 0; i < BITMAP_SIZE * 8; i++) {
        if (gc_bitmap_get(alloc_bitmap, i) && !gc_bitmap_get(marked_bitmap, i)) {
            gc_bitmap_set(alloc_bitmap, i, 0);
            num_small_sweeped++;
        }
    }
    hoppus_printf("INFO: GC: sweeped %d objects\n", num_sweeped);
    hoppus_printf("INFO: GC: sweeped %d small objects\n", num_small_sweeped);
    return 0;
}

__USER_TEXT int gc_dump_info() {
    int count = 0, marked = 0, unmarked = 0;
    for (header *u = large_used; u != NULL; u = UNTAG(u->next)) {
        count++;
        if (((uint32_t)u->next & 1) == 1) {
            marked++;
        } else {
            unmarked++;
        }
    }
    int small_count = 0, small_marked = 0, small_unmarked = 0;
    for (int i = 0; i < BITMAP_SIZE * 8; i++) {
        if (gc_bitmap_get(alloc_bitmap, i)) {
            small_count++;
            if (gc_bitmap_get(marked_bitmap, i)) {
                small_marked++;
            } else {
                small_unmarked++;
            }
        }
    }
    hoppus_printf("INFO: GC: %d allocated objects\n", count);
    hoppus_printf("INFO: GC: %d marked objects\n", marked);
    hoppus_printf("INFO: GC: %d unmarked objects\n", unmarked);
    hoppus_printf("INFO: GC: %d small allocated objects\n", small_count);
    hoppus_printf("INFO: GC: %d small marked objects\n", small_marked);
    hoppus_printf("INFO: GC: %d small unmarked objects\n", small_unmarked);
    return 0;
}

__USER_TEXT int gc_mark_and_sweep() {
    uint32_t *sp;
    register void *_sp asm ("sp");
    sp = _sp;

    hoppus_puts("-----------------------------\n");
    hoppus_puts("INFO: GC: starting\n");

    /* Reset gc marks */
    for (header *u = large_used; u != NULL; u = UNTAG(u->next))
        u->next = UNTAG(u->next);
    for (int i = 0; i < BITMAP_SIZE * 8; i++)
        gc_bitmap_set(marked_bitmap, i, 0);

    if (large_used == NULL) {
        return 0;
    }

#ifdef HOPPUS_PLATFORM
#if HOPPUS_PLATFORM == HOPPUS_X86
    /* provided by linker, sdata is from customized default linker script,
      not in regular default linker script */
    extern char end, sdata;
    gc_scan_region((uint32_t *)&sdata, (uint32_t *)malloc_heap);
    gc_scan_region((uint32_t *)(malloc_heap + MALLOC_HEAP_SIZE), (uint32_t *)&end - 1);
#elif HOPPUS_PLATFORM == HOPPUS_RISCV_F9
    gc_scan_region((uint32_t *)&user_bss_start, (uint32_t *)&user_bss_end);
    gc_scan_region((uint32_t *)&user_data_misc_start, (uint32_t *)&user_data_misc_end);
#endif
#endif
    gc_scan_region(sp, (uint32_t *)stack_end);

    hoppus_puts("INFO: GC: scanning marked objects...\n");
    gc_scan_heap();


    hoppus_puts("INFO: GC: scan finished\n");
    gc_dump_info();
    hoppus_printf("INFO: GC: %d / %d bytes allocated\n", gc_allocated_size, heap_size);
    hoppus_printf("INFO: GC: %d / %d recalculated\n", gc_calc_allocated(), heap_size);
    /* gc_bitmap_print(alloc_bitmap); */
    gc_sweep();
    /* gc_bitmap_print(alloc_bitmap); */
    hoppus_printf("INFO: GC: %d num mallocs\n", gc_stats_get_num_malloc());
    hoppus_printf("INFO: GC: %d / %d bytes allocated\n", gc_allocated_size, heap_size);
    hoppus_printf("INFO: GC: %d / %d recalculated\n", gc_calc_allocated(), heap_size);
    hoppus_puts("-----------------------------\n");

    return 0;
}

__USER_TEXT void gc_alloc_dump() {
    hoppus_printf("INFO: GC: heap start %p, end %p\n", heap_start, heap_end);
    int i = 0;
    for (header *u = large_used, *tag_addr = large_used->next;
         u != NULL && UNTAG(u->next) != NULL;
         u = UNTAG(u->next), tag_addr = u->next) {
        hoppus_printf("INFO: GC: %d: tag_addr: %p, %p -> %p, size: %lu\n", i++, tag_addr, u, u + u->size, u->size * sizeof(header));
    }
}

__USER_TEXT void *gc_malloc_units(unsigned int required_units) {
    /* Set up used list if it is empty */
    if (large_used == NULL) {
        header *free_base = (header *)heap_split;
        free_base->size = required_units - 1;
        free_base->next = NULL;
        large_used = free_base;
        large_prev_malloced = free_base;
        gc_allocated_size += (free_base->size + 1) * sizeof(header);
        return free_base + 1;
    }

    if (large_prev_malloced == NULL) {
        large_prev_malloced = large_used;
    }
    int first_round = 1;
    for (header *u = UNTAG(large_prev_malloced);; u = UNTAG(u->next)) {
        if (!first_round && u == large_prev_malloced)
            break;
        first_round = 0;
        if (u == NULL)
            u = large_used;

        header *free_base, *u_end = u + 1 + u->size;

        /* Space between this block and the next block. Most common case, so lets check for it first. */
        if (UNTAG(u->next) != NULL && required_units < UNTAG(u->next) - u_end) {
            free_base = u_end;
            free_base->next = u->next;
            u->next = new_next_ptr(u->next, free_base); // TODO: Why do we not use new_next_ptr here?
        /* Start of the used list, and space before first block */
        } else if (u == large_used && required_units < u - (header *)heap_split) {
            free_base = (header *)heap_split;
            free_base->next = u;
            large_used = free_base;
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
        large_prev_malloced = free_base;
        gc_allocated_size += (free_base->size + 1) * sizeof(header);
        return free_base + 1;
    }
    return NULL;
}


__USER_TEXT void *gc_malloc_large(unsigned int size) {

    unsigned int required_units = (align_up(size, sizeof(header)) + sizeof(header)) / sizeof(header);
    if (required_units * sizeof(header) > heap_size) {
        hoppus_printf("ERROR: GC: MALLOC: got request to allocate %d bytes, which means I need to allocate a total of %d, but its more than the size of the heap which is %d\n",
               size,
               required_units * sizeof(header),
               heap_size);
        return (void *)NULL;
    }

    void *ptr = gc_malloc_units(required_units);

    return ptr;
}


__USER_TEXT void *gc_malloc_small() {
    /* Jump 32 bits at a time */
    int first_round = 1;
    for (int i = small_prev_malloced_byte;; i += 4) {
        if (first_round) {
            first_round = 0;
        } else if (i == small_prev_malloced_byte) {
            break;
        }
        if (i == BITMAP_SIZE) {
            i = 0;
            continue;
        }
        /* If all bits are taken in this 32 bit area, go to next */
        if (((uint32_t*)alloc_bitmap)[i / 4] == 0xFFFFFFFF)
            continue;
        for (int j = i * 8; j < (i * 8) + 32; j++) {
            if (!gc_bitmap_get(alloc_bitmap, j)) {
                gc_bitmap_set(alloc_bitmap, j, 1);
                small_prev_malloced_byte = i;
                return &((small_obj *)heap_start)[j];
            }
        }
        hoppus_printf("ERROR: GC: couldnt find available slot in 32-bit region that should be available %x, %d\n", ((uint32_t*)alloc_bitmap)[i / 4], i)
    }
    return NULL;
}

__USER_TEXT void *gc_malloc(unsigned int size) {
    if (size <= 0) {
        hoppus_puts("ERROR: GC: MALLOC: got request to malloc with size 0\n");
        return (void *)NULL;
    }
    if (!heap_start || !heap_split || !heap_end) {
        hoppus_puts("ERROR: GC: MALLOC: missing heap start or end\n");
        return (void *)NULL;
    }

    void *ptr;
    if (size == sizeof(small_obj)) {
        ptr = gc_malloc_small();
    } else {
        ptr = gc_malloc_large(size);
    }

    if (ptr == NULL) {
        /* no space, run gc, then try again */
        gc_mark_and_sweep();
        if (size == sizeof(small_obj)) {
            ptr = gc_malloc_small();
        } else {
            ptr = gc_malloc_large(size);
        }
    }

    if (ptr == NULL) {
        hoppus_printf("ERROR: GC: MALLOC: couldnt find space for size %d\n", size);
    }
    gc_stats_num_malloc += 1;
    return ptr;
}
