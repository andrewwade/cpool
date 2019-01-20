//
// Created by Andrew Wade on 2019-01-18.
//

#include <assert.h>
#include "stdio.h"
#include "stdint.h"

#define ERROR(code, fmt, ...) do {  \
    printf("Error(%d): ", code);  \
    printf(fmt,##__VA_ARGS__);      \
    printf("\n");                   \
} while(0)

#include "byte_pool.c"


void byte_pool_debug_dump(byte_pool_t *pool);


void byte_pool_debug_dump(byte_pool_t *pool) {
    byte_header_t *curr = pool->start;

    while (curr != NULL) {
        printf("\n");
        printf("Address: %p\n", (void *) curr);
        printf("   Next: %p\n", (void *) byte_block_get_next(curr));
        printf("   Free: %d\n", byte_block_is_free(curr));
        printf("   Size: %lu\n", byte_block_get_size(&curr));

        curr = byte_block_get_next(curr);
    }
}

void test_block_get_size() {
    byte_header_t header = {.next = &header+3,.owner=NULL};
    byte_header_t *ptr = &header;
    size_t size = byte_block_get_size(&ptr);
    assert(size == 32);
}

void test_byte_pool_run() {
    byte_pool_t pool;
    uint8_t buffer[512];

    test_block_get_size();
    byte_pool_init(&pool, buffer, 512);

    void *data = byte_allocate(&pool, 32);
    void *data2= byte_allocate(&pool, 16);

    byte_pool_debug_dump(&pool);

    byte_release(data);
    byte_pool_debug_dump(&pool);

    byte_release(data2);
    printf("\nExpected Error: ");
    byte_release(data2);
    byte_pool_defragment(&pool);
    byte_pool_debug_dump(&pool);
}