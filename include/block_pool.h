//
// Created by Andrew Wade on 2019-01-12.
//

#ifndef MEMORY_BLOCK_POOL_H
#define MEMORY_BLOCK_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct block_pool_t {
    void *memory;
    void *available;
    unsigned alignment;
    unsigned size;
} block_pool_t;

void block_pool_init(block_pool_t *pool, unsigned block_size, void *memory, unsigned memory_size);

void block_pool_destroy(block_pool_t *pool);

void *block_allocate(block_pool_t *pool);

void block_free(void *block);



#ifdef __cplusplus
};
#endif

#endif //MEMORY_BLOCK_POOL_H
