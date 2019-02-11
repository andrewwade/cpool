//
// Created by Andrew Wade on 2019-01-20.
//

#include "segment_pool.h"
#include <stdbool.h>

static void segment_pool_segment(void *memory, size_t alignment, size_t size, bool null_ending);

void segment_pool_init(segment_pool_t *pool, size_t alignment, void *start, void *end) {
    pool->alignment = (alignment < sizeof(void*)) ? sizeof(void*) : alignment;
    pool->start = start;
    pool->search = start;
    pool->end = end;

    segment_pool_segment(pool->start, pool->alignment, pool->end-pool->start, true);
    pool->search = pool->start;
}

int segment_pool_empty(segment_pool_t *pool) {
    return (pool == NULL) || (pool->start == NULL) ||(pool->search == NULL);
}

void *segment_allocate(struct segment_pool_t *pool) {
    void *return_ptr = pool->search;
    if(pool->search) {
        pool->search = *(void **)pool->search;
    }
    return return_ptr;
}

void segment_release(struct segment_pool_t *pool, void *memory) {
    *(char **)memory = pool->search;
    pool->search = memory;
}

void segment_ordered_release(struct segment_pool_t *pool, void *memory) {
    void *search = pool->search;
    void *next;

    while(search != NULL) {
        next = *(char**)search;
        if(search < memory && memory < next) {
            *(char**)memory = next;
            *(char**)search = memory;
            search = NULL; /*done searching */
        } else {
            search = next;
        }
    }
}

void *segment_allocate_size(struct segment_pool_t *pool, size_t size) {
    void *search = pool->search;
    void *next;
    void *return_ptr= NULL;
    size_t available = 0;

    while(search != NULL && available < size) {
        next = *(char**)search;
        if(next == search+pool->alignment) {
            /* this is free */
            available += pool->alignment;
        } else {
            available = 0;
        }
        search = next;
    }

    if(available >= size) {
        pool->search = search;
        return_ptr = search-available;
    }

    return return_ptr;
}

void segment_release_size(struct segment_pool_t *pool, void *memory, size_t size) {
    segment_pool_segment(memory, pool->alignment, size - pool->alignment, true);
    *(char**)(memory+size-pool->alignment) = pool->search;
    pool->search = memory;
}

void segment_ordered_release_size(struct segment_pool_t *pool, void *memory, size_t size) {
    void *search = pool->search;
    void *next;

    while(search != NULL) {
        next = *(char**)search;
        if(search < memory && memory < next) {
            segment_pool_segment(memory, pool->alignment, size - pool->alignment, true);
            *(char**)(memory+size-pool->alignment) = pool->search;
            pool->search = memory;
            search = NULL; /*done searching */
        } else {
            search = next;
        }
    }
}

static void segment_pool_segment(void *memory, size_t alignment, size_t size, bool null_ending) {
    for(size_t i = 0; i < size; i += alignment) {
        *(char**)memory = memory+alignment;
        memory = *(char**)memory;
    }
    if(null_ending) {
        *(char **) memory = NULL; /* null terminated end */
    }
}