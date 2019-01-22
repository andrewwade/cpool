//
// Created by Andrew Wade on 2019-01-20.
//

#include "assert.h"
#include <memory_pool.h>
#include "test_memory_pool.h"

void test_memory_pool_run() {
    int buffer[512];

    memory_pool_t buffer_pool;
    memory_pool_t *pool = &buffer_pool;

    memory_pool_init(pool, sizeof(int), buffer, buffer + 512);

    int *v1 = memory_pool_allocate(pool);

    int *v4 = memory_pool_allocate_size(pool, sizeof(int) * 4);

    memory_pool_release_size(pool, v4, sizeof(int) * 4);
}