//
// Created by Andrew Wade on 2019-01-20.
//

#include "assert.h"
#include <chunk_pool.h>

void test_chunk_pool_run() {
    int buffer[512];

    chunk_pool_t buffer_pool;
    chunk_pool_t *pool = &buffer_pool;

    chunk_pool_init(pool, sizeof(int), buffer, buffer + 512);

    int *v1 = (int*)chunk_allocate(pool);

    int *v4 = (int*)chunk_allocate_size(pool, sizeof(int) * 4);

    chunk_release_size(pool, v4, sizeof(int) * 4);
}