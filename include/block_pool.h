//
// Created by Andrew Wade on 2019-01-12.
//

#ifndef MEMORY_BLOCK_POOL_H
#define MEMORY_BLOCK_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct block_pool_t {
    void *start;
    void *search;
    void *end;
    size_t alignment;
    size_t capacity;
    size_t available;
} block_pool_t;

void block_pool_init(block_pool_t *pool, size_t alignment, void *start, void *end);

int block_pool_is_valid(block_pool_t *pool);

void block_pool_reset(block_pool_t *pool, size_t alignment);

void *block_allocate(block_pool_t *pool);

void block_release(void *block);

#ifdef __cplusplus
};
#endif

#endif //MEMORY_BLOCK_POOL_H
