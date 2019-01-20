//
// Created by Andrew Wade on 2019-01-20.
//

#include <stdbool.h>
#include "memory_pool.h"

static void memory_pool_segment(void *memory, size_t alignment, size_t size, int null_ending);

void memory_pool_init(memory_pool_t *pool, size_t alignment, void *start, void *end) {
    pool->alignment = (alignment < sizeof(void*)) ? sizeof(void*) : alignment;
    pool->start = start;
    pool->search = start;
    pool->end = end;
    pool->capacity = (end-start)/alignment;
    pool->fragments = 0;

    memory_pool_segment(pool->start, pool->alignment, pool->end-pool->start, true);
    pool->search = pool->start;
}


int memory_pool_empty(memory_pool_t *pool) {
    return (pool == NULL) || (pool->start == NULL) ||(pool->search == NULL);
}

void *memory_pool_allocate(memory_pool_t *pool) {
    void *return_ptr = pool->search;
    if(pool->search) {
        pool->search = *(void **)pool->search;
    }
    return return_ptr;
}

void memory_pool_release(memory_pool_t *pool, void *memory) {
    *(char **)memory = pool->search;
    pool->search = memory;
}

void memory_pool_ordered_release(memory_pool_t *pool, void *memory) {
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

void *memory_pool_allocate_size(memory_pool_t *pool, size_t size) {
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

void memroy_pool_release_size(memory_pool_t *pool, void *memory, size_t size) {
    memory_pool_segment(memory, pool->alignment, size - pool->alignment, true);
    *(char**)(memory+size-pool->alignment) = pool->search;
    pool->search = memory;
}

void memory_pool_ordered_release_size(memory_pool_t *pool, void *memory, size_t size) {
    void *search = pool->search;
    void *next;

    while(search != NULL) {
        next = *(char**)search;
        if(search < memory && memory < next) {
            memory_pool_segment(memory, pool->alignment, size - pool->alignment, true);
            *(char**)(memory+size-pool->alignment) = pool->search;
            pool->search = memory;
            search = NULL; /*done searching */
        } else {
            search = next;
        }
    }
}

static void memory_pool_segment(void *memory, size_t alignment, size_t size, int null_ending) {
    for(size_t i = 0; i < size; i += alignment) {
        *(char**)memory = memory+alignment;
        memory = *(char**)memory;
    }
    if(null_ending) {
        *(char **) memory = NULL; /* null terminated end */
    }
}