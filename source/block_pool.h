//
// Created by Andrew Wade on 2019-01-12.
//

#ifndef MALLOC_OVERRIDE_TEST_BLOCK_POOL_H
#define MALLOC_OVERRIDE_TEST_BLOCK_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct block_pool_t {
    void *memory;
    void *available;
    unsigned alignment;
    unsigned size;
} block_pool_t;

void block_pool_init(block_pool_t *pool, unsigned alignment, void *memory, unsigned size);

void block_pool_destroy(block_pool_t *pool);

void *block_allocate(block_pool_t *pool);

void block_free(void *block);



#ifdef __cplusplus
};
#endif

#endif //MALLOC_OVERRIDE_TEST_BLOCK_POOL_H
