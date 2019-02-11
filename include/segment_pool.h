//
// Created by Andrew Wade on 2019-01-20.
//

#ifndef MEMORY_SEGMENT_POOL_H
#define MEMORY_SEGMENT_POOL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct segment_pool_t segment_pool_t;

struct segment_pool_t {
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
void segment_pool_init(segment_pool_t *pool, size_t alignment, void *start, void *end);

/**
 * Allocate single segment
 * @param pool
 * @return
 */
void *segment_allocate(segment_pool_t *pool);

/**
 * Unordered release of single segment
 * @param pool
 * @param memory
 */
void segment_release(segment_pool_t *pool, void *memory);

/**
 * Ordered release of single segment
 * @note Recommended if using custom sized segments  to reduce defragmentation
 * @param pool
 * @param memory
 */
void segment_ordered_release(segment_pool_t *pool, void *memory);

/**
 * Allocate custom size from pool
 * @param pool
 * @param size  number of bytes to allocate
 * @return pointer to allocated memory. Null if not enough memory available
 */
void *segment_allocate_size(segment_pool_t *pool, size_t size);

/**
 * Unordered release of custom sized segments
 * @note    Worse defragmentation than ordered release
 * @param pool      original owner of the memory
 * @param memory    memory to release
 * @param size      size of memory segment
 */
void segment_release_size(segment_pool_t *pool, void *memory, size_t size);

/**
 * Ordered release of custom sized segments
 * @note    Recommended to reduce defragmentation
 * @param pool      original owner of the memory
 * @param memory    memory to release
 * @param size      size of memory segment
 */
void segment_ordered_release_size(segment_pool_t *pool, void *memory, size_t size);

/**
 * Check if pool is empty
 * @param pool
 * @return
 */
int segment_pool_empty(segment_pool_t *pool);

#ifdef __cplusplus
};
#endif

#endif //TESTABLE_MEMORY_POOL_H
