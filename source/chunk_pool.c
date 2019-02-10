//
// Created by Andrew Wade on 2019-01-20.
//

#include "chunk_pool.h"
#include <stdbool.h>

static void chunk_pool_segment(void *memory, size_t alignment, size_t size, bool null_ending);

void chunk_pool_init(chunk_pool_t *pool, size_t alignment, void *start, void *end) {
    pool->alignment = (alignment < sizeof(void*)) ? sizeof(void*) : alignment;
    pool->start = start;
    pool->search = start;
    pool->end = end;

    chunk_pool_segment(pool->start, pool->alignment, pool->end-pool->start, true);
    pool->search = pool->start;
}


int chunk_pool_empty(chunk_pool_t *pool) {
    return (pool == NULL) || (pool->start == NULL) ||(pool->search == NULL);
}

void *chunk_allocate(chunk_pool_t *pool) {
    void *return_ptr = pool->search;
    if(pool->search) {
        pool->search = *(void **)pool->search;
    }
    return return_ptr;
}

void chunk_release(chunk_pool_t *pool, void *memory) {
    *(char **)memory = pool->search;
    pool->search = memory;
}

void chunk_ordered_release(chunk_pool_t *pool, void *memory) {
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

void *chunk_allocate_size(chunk_pool_t *pool, size_t size) {
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

void chunk_release_size(chunk_pool_t *pool, void *memory, size_t size) {
    chunk_pool_segment(memory, pool->alignment, size - pool->alignment, true);
    *(char**)(memory+size-pool->alignment) = pool->search;
    pool->search = memory;
}

void chunk_ordered_release_size(chunk_pool_t *pool, void *memory, size_t size) {
    void *search = pool->search;
    void *next;

    while(search != NULL) {
        next = *(char**)search;
        if(search < memory && memory < next) {
            chunk_pool_segment(memory, pool->alignment, size - pool->alignment, true);
            *(char**)(memory+size-pool->alignment) = pool->search;
            pool->search = memory;
            search = NULL; /*done searching */
        } else {
            search = next;
        }
    }
}

static void chunk_pool_segment(void *memory, size_t alignment, size_t size, bool null_ending) {
    for(size_t i = 0; i < size; i += alignment) {
        *(char**)memory = memory+alignment;
        memory = *(char**)memory;
    }
    if(null_ending) {
        *(char **) memory = NULL; /* null terminated end */
    }
}