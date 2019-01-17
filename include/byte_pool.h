//
// Created by Andrew Wade on 2019-01-12.
//

#ifndef MEMORY_BYTE_POOL_H
#define MEMORY_BYTE_POOL_H

#ifdef __cplusplus
extern "C" {
#endif
    

#define BYTE_BLOCK_MIN 16

typedef struct byte_pool_t {
    void *memory;
    void *iter;
    unsigned size;
    unsigned bytes_available;
    unsigned fragments;
} byte_pool_t;

void byte_pool_init(byte_pool_t *pool, void *memory, unsigned size);

byte_pool_t *byte_pool_create(unsigned size);

void byte_pool_destroy(byte_pool_t *pool);

void byte_pool_defragment(byte_pool_t *pool);

void *byte_pool_allocate(byte_pool_t *pool, unsigned size);

void byte_pool_free(void *memory);

void byte_pool_debug_dump(byte_pool_t *pool);
#ifdef __cplusplus
};
#endif

#endif //MEMORY_BYTE_POOL_H
