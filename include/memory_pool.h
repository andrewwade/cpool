//
// Created by Andrew Wade on 2019-01-20.
//

#ifndef TESTABLE_MEMORY_POOL_H
#define TESTABLE_MEMORY_POOL_H


#ifdef __cplusplus
extern "C" {
#endif
#include "stddef.h"


typedef struct memory_pool_t memory_pool_t;

struct memory_pool_t {
    void *start;
    void *search;
    void *end;
    size_t alignment;
    size_t capacity;
    size_t fragments;
};

/**
 *
 * @param pool
 * @param alignment Must be >= sizeof(void*)
 * @param start
 * @param end
 */
void memory_pool_init(memory_pool_t *pool, size_t alignment, void *start, void *end);

void *memory_pool_allocate(memory_pool_t *pool);

void memory_pool_release(memory_pool_t *pool, void *memory);

void memory_pool_ordered_release(memory_pool_t *pool, void *memory);

void *memory_pool_allocate_size(memory_pool_t *pool, size_t size);

void memroy_pool_release_size(memory_pool_t *pool, void *memory, size_t size);

void memory_pool_ordered_release_size(memory_pool_t *pool, void *memory, size_t size);

int memory_pool_empty(memory_pool_t *pool);

#ifdef __cplusplus
};
#endif

#endif //TESTABLE_MEMORY_POOL_H
