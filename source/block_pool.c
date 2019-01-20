//
// Created by Andrew Wade on 2019-01-12.
//

#include <stddef.h>
#include "block_pool.h"

typedef union block_header_t block_header_t;

union block_header_t {
    block_header_t *next;
    block_pool_t   *owner;
};

void block_pool_init(block_pool_t *pool, size_t alignment, void *start, void *end) {
    if (pool != NULL) {
        if ((end - start) > sizeof(block_header_t) + alignment) {
            pool->start     = start;
            pool->search    = pool->start;
            pool->end       = end;
            pool->alignment = alignment;
            pool->capacity  = 0;
            pool->available = 0;

            block_pool_reset(pool, pool->alignment);
        }
    }
}

int block_pool_is_valid(block_pool_t *pool) {
    return (pool != NULL) &&
           (pool->start < pool->end) &&
           (pool->capacity <= (pool->end - pool->start));
}

void block_pool_reset(block_pool_t *pool, size_t alignment) {
    block_header_t *block;
    if (block_pool_is_valid(pool)) {
        if (pool->available == pool->capacity) {
            pool->alignment = alignment;

            /* initialize memory into a stack where each element points to the next element*/
            for (block = pool->search; (void *) (block + 1) + alignment < pool->end; block = block->next) {
                block->next = (void *) (block + 1) + alignment;
                pool->capacity++;
            }

            /* set last element value to NULL to indicate end of memory*/
            block->next = NULL;

            /* set all blocks available */
            pool->available = pool->capacity;
        }
    }
}

void *block_allocate(block_pool_t *pool) {
    block_header_t *block = NULL;
    if (block_pool_is_valid(pool)) {
        if(pool->search != NULL && pool->available > 0) {
            block = pool->search;
            pool->search = block->next;
            block->owner = pool;
            pool->available--;
            block = block + 1; /* move block ptr to user space */
        }
    }
    return block;
}

void block_release(void *memory) {
    block_header_t *block;
    block_pool_t   *pool;

    if (memory != NULL) {
        /* get block ptr */
        block = ((block_header_t *)memory) - 1;
        pool  = block->owner;

        if (block_pool_is_valid(pool)) {
            block->next  = pool->search;
            pool->search = block;
        }
    }
}