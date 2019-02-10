//
// Created by Andrew Wade on 2019-01-20.
//

#ifndef TESTABLE_MEMORY_POOL_H
#define TESTABLE_MEMORY_POOL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct chunk_pool_t chunk_pool_t;

struct chunk_pool_t {
    void *start;
    void *search;
    void *end;
    size_t alignment;
};

/**
 *
 * @param pool
 * @param alignment >= sizeof(void*)
 * @param start
 * @param end
 */
void chunk_pool_init(chunk_pool_t *pool, size_t alignment, void *start, void *end);

/**
 * Allocate single alignment chunk
 * @param pool
 * @return
 */
void *chunk_allocate(chunk_pool_t *pool);

/**
 * Unordered release of single alignment chunk
 * @param pool
 * @param memory
 */
void chunk_release(chunk_pool_t *pool, void *memory);

/**
 * Ordered release of single alignment chunk
 * @note Recommended if using variable sized chunks to prevent defragmentation
 * @param pool
 * @param memory
 */
void chunk_ordered_release(chunk_pool_t *pool, void *memory);

/**
 * Allocate custom size from pool
 * @note Recommend to use ordered release to reduce defragmentation
 * @param pool
 * @param size  number of bytes to allocate
 * @return pointer to allocated memory. Null if not enough memory available
 */
void *chunk_allocate_size(chunk_pool_t *pool, size_t size);

/**
 * Unordered release of variable sized memory block
 * @note    It is recommended to use the ordered release instead to reduce defragmentation
 * @param pool      original owner of the memory
 * @param memory    memory to release
 * @param size      size of memory block
 */
void chunk_release_size(chunk_pool_t *pool, void *memory, size_t size);

/**
 * Ordered release of variable sized memory block
 * @note    Recommended to reduce defragmentation
 * @param pool      original owner of the memory
 * @param memory    memory to release
 * @param size      size of memory block
 */
void chunk_ordered_release_size(chunk_pool_t *pool, void *memory, size_t size);

/**
 * Check if pool is empty
 * @param pool
 * @return
 */
int chunk_pool_empty(chunk_pool_t *pool);

#ifdef __cplusplus
};
#endif

#endif //TESTABLE_MEMORY_POOL_H
