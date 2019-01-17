#include <stdio.h>
#include <stdbool.h>

#include "stdlib.h"
#include "byte_pool.h"
#include "block_pool.h"

#define BYTE_BLOCK_MIN 16

typedef unsigned char byte_t;

uint8_t buffer[512];

int main() {
    byte_pool_t pool;
    byte_pool_init(&pool, buffer, 512);

    void *data = byte_pool_allocate(&pool, 32);

    byte_pool_allocate(&pool, 16);

    byte_pool_debug_dump(&pool);

    byte_pool_free(data);

    byte_pool_debug_dump(&pool);

    byte_pool_destroy(&pool);

    block_pool_t block_pool;

    block_pool_init(&block_pool, 32, buffer, 512);

    data = block_allocate(&block_pool);

    block_allocate(&block_pool);
    block_free(data);
    return 0;
}