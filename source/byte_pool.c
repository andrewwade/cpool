//
// Created by Andrew Wade on 2019-01-12.
//

#include <stddef.h>
#include "byte_pool.h"
#include <stdlib.h>
#include <printf.h>

//typedef struct block_header_t {
//    uintptr_t next : (sizeof(uintptr_t) * 8) - 1;
//    unsigned  allocated : 1;
//} block_header_t;

#ifndef ERROR
#define ERROR(code, str...)
#endif

typedef struct block_header_t block_header_t;
typedef block_header_t        *block_t;

struct block_header_t {
    block_t     next;
    byte_pool_t *owner;
};


size_t block_get_size(block_t *block) {
    return ((*block)->next != 0) ? (uintptr_t)(*block)->next - (uintptr_t)(*block + 1) : 0;
}

void block_merge_next(block_t *block) {
    block_t head, next;
    head = *block;
    next = (block_t) head->next;
    head->next = next->next;
}

int block_needs_split(block_t *block, size_t size) {
    return (block_get_size(block) - size > BYTE_BLOCK_MIN);
}

void block_split(block_t *block, size_t size) {
    block_t head, split, tail;

    if (block_needs_split(block, size)) {
        head  = *block;
        tail  = (block_t) head->next;
        split = (block_t) ((uintptr_t) (head + 1) + size);
        head->next  = split;
        split->next = tail;
    }
}

void *byte_block_allocate(byte_pool_t *pool, block_t *block, size_t size) {
    void *return_ptr = NULL;

    if (block_needs_split(block, size)) {
        block_split(block, size);
        pool->fragments++;
    }

    if (block_get_size(block) >= size) {
        (*block)->owner = pool;
        return_ptr = *block + 1;
    }

    return return_ptr;
}

block_t block_get_next(block_t block) {

    if (block == NULL || ((block_t) (block->next))->next == NULL) {
        /* next block is not valid (reached the end) */
        return NULL;
    }
    return (block_t) (block->next);
}

int block_is_free(block_t block) {
    return (block != NULL) ? block->owner == NULL : 0;
}

int block_is_allocated(block_t block) {
    return (block != NULL) ? block->owner != NULL : 0;
}


void byte_pool_init(byte_pool_t *pool, void *memory, unsigned size) {
    block_t header;

    pool->memory          = memory;
    pool->iter            = memory;
    pool->size            = size;
    pool->fragments       = 1;
    pool->bytes_available = size - sizeof(block_header_t);

    header = pool->memory;
    byte_block_allocate(pool, &header, pool->bytes_available);
    header->owner = NULL;
}

int pool_created_from_malloc(byte_pool_t *pool) {
    return (pool + 1 == pool->memory);
}

byte_pool_t *byte_pool_create(unsigned size) {
    byte_pool_t *pool = malloc(sizeof(byte_pool_t) + size);
    byte_pool_init(pool, pool + 1, size);
    return pool;
}

void byte_pool_destroy(byte_pool_t *pool) {
    if (pool_created_from_malloc(pool)) {
        free(pool);
    } else {
        pool->iter            = NULL;
        pool->size            = 0;
        pool->bytes_available = 0;
        pool->fragments       = 0;
        pool->memory          = NULL;
    }
}

void byte_pool_defragment(byte_pool_t *pool) {

}

void *byte_pool_allocate(byte_pool_t *pool, unsigned size) {
    void    *return_ptr;
    block_t block;
    block_t next;
    size_t  available_bytes;
    size_t  fragments_to_examine;


    block                = pool->iter;
    available_bytes      = 0;
    fragments_to_examine = pool->fragments + 1;


    do {
        if (block_is_free(block)) {
            /* block is free */
            available_bytes = block_get_size(&block);

            if (available_bytes >= size) {
                /* block is free and big enough. stop searching */
                break;
            } else {
                available_bytes = 0;
                next = block_get_next(block);
                if(next != NULL) {
                    if (block_is_free(next)) {
                        /* next block is free, merge them then re-evaluate */
                        block_merge_next(&block);

                        if (pool->iter == next) {
                            pool->iter = block;
                        }
                    } else {
                        /* next block is not free, skip it */
                        block = block_get_next(next);
                        if (fragments_to_examine) {
                            /* that's one less block to check */
                            fragments_to_examine--;
                        }
                    }
                }
            }
        } else {
            block = block_get_next(block);
        }

        if (fragments_to_examine) {
            fragments_to_examine--;
        }

    } while (fragments_to_examine);

    if (available_bytes) {
        return_ptr = byte_block_allocate(pool, &block, size);

        pool->bytes_available -= (size + sizeof(void *));
    } else {
        return_ptr = NULL;
    }
    return return_ptr;
}

void byte_pool_free(void *memory) {
    block_t header = ((block_t) memory) - 1;
    if (header->owner != NULL) {
        byte_pool_t *pool = header->owner;
        if (pool->memory <= memory && pool->memory + pool->size > memory) {
            header->owner = NULL;
            pool->bytes_available += block_get_size(&header) + sizeof(block_header_t);
        } else {
            ERROR(14, "Byte Pool Pointer Error");
        }

    } else {
        ERROR(14, "Byte Pool Pointer Error");
    }
}


void byte_pool_debug_dump(byte_pool_t *pool) {
    block_t curr = pool->memory;

    while (curr != NULL) {
        printf("\n");
        printf("Address: %p\n", (void *) curr);
        printf("   Next: %p\n", (void *) block_get_next(curr));
        printf("   Free: %d\n", block_is_free(curr));
        printf("   Size: %lu\n", block_get_size(&curr));

        curr = block_get_next(curr);
    }
}