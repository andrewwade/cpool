//
// Created by Andrew Wade on 2019-01-12.
//

#ifndef MEMORY_BYTE_POOL_H
#define MEMORY_BYTE_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"

#ifndef BYTE_BLOCK_MIN
#define BYTE_BLOCK_MIN 16 /* set minimum byte block size to reduce fragmentation */
#endif

typedef struct byte_pool_t {
    void *start;
    void *search;
    void *end;
    size_t capacity;
    size_t fragments;
} byte_pool_t;

void byte_pool_init(byte_pool_t *pool, void *memory, size_t size);

int byte_pool_is_valid(byte_pool_t *pool);

void byte_pool_defragment(byte_pool_t *pool);

void *byte_allocate(byte_pool_t *pool, size_t size);

void byte_release(void *memory);

#ifdef __cplusplus
};
#endif

#endif //MEMORY_BYTE_POOL_H
