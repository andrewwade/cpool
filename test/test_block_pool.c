//
// Created by Andrew Wade on 2019-01-18.
//

#include <stdint.h>
#include "block_pool.c"

void test_block_pool_run() {
    block_pool_t block_pool;
    uint8_t buffer[512];
    void *data;

    block_pool_init(&block_pool, 32, buffer, buffer+512);

    data = block_allocate(&block_pool);

    block_allocate(&block_pool);
    block_release(data);
}