//
// Created by Andrew Wade on 2019-01-12.
//

#include <stddef.h>
#include "block_pool.h"
#include "stdlib.h"



typedef union block_header_t block_header_t;
typedef block_header_t * block_t;


union block_header_t {
    block_header_t *next;
    block_pool_t *owner;
};

void block_pool_init(block_pool_t *pool, unsigned alignment, void *memory, unsigned size) {
    if (pool == NULL) { return; }
    if (size < alignment + sizeof(block_header_t)) { return; }

    pool->alignment = alignment;
    pool->size      = size;
    pool->memory    = memory;

    /* create memory memory if needed*/
    if (pool->memory == NULL && (pool->memory = malloc(pool->size)) == NULL) {
        return;
    }

    /* assign head to front of memory */
    pool->available = pool->memory;

    /* initialize memory into a stack where each element points to the next element*/
    char *element;
    for (element = pool->available; size > alignment; element += alignment+sizeof(block_header_t), size -= (alignment+sizeof(block_header_t))) {
        *(void **) element = element + alignment+sizeof(block_header_t);
    }

    /* set last element value to NULL to indicate end of memory*/
    *(void **) element = NULL;

}

void block_pool_destroy(block_pool_t *pool) {
    pool->available = NULL;
    pool->memory = NULL;
    pool->size = 0;
    pool->alignment = 0;
}

void *block_allocate(block_pool_t *pool) {
    block_t header = pool->available;
    pool->available = header->next;
    header->owner = pool;
    return header+1;
//    return (void*)(uintptr_t)header+sizeof(block_header_t);
}

void block_free(void *block) {
    if(block != NULL) {
        block_t header = (block_t)block - 1;
        block_pool_t *pool = header->owner;
        if(pool != NULL && pool->memory < block && block < pool->memory+pool->size) {
            header->next = pool->available;
            pool->available = header;
        }
    }
}