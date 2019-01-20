//
// Created by Andrew Wade on 2019-01-12.
//

#include "byte_pool.h"
#include <stdbool.h>

#ifndef ERROR
#define ERROR(code, str...)
#endif

typedef struct byte_header_t byte_header_t;

struct byte_header_t {
    byte_header_t *next;
    byte_pool_t   *owner;
};

static byte_header_t *get_header_from_memory(void *memory) {
    return ((byte_header_t *) memory) - 1;
}

static bool byte_block_is_free(byte_header_t *block) {
    return (block != NULL) && block->owner == NULL;
}

static bool byte_block_is_valid(byte_header_t *block) {
    return (block != NULL) && (block->next != NULL);
}

static size_t byte_block_get_size(byte_header_t **block) {
    size_t size = 0;
    if (byte_block_is_valid(*block)) {
        size = ((void *) (*block)->next - (void *) (*block + 1));
    }
    return size;
}

static void byte_block_merge_next(byte_pool_t *pool, byte_header_t *block) {
    byte_header_t *next = block->next;
    block->next = next->next;
    pool->fragments--;
    if (pool->search == next) {
        pool->search = block;
    }
}

static int byte_block_needs_split(byte_header_t **block, size_t size) {
    return (byte_block_get_size(block) - size > BYTE_BLOCK_MIN);
}

static void byte_block_split(byte_header_t **block, size_t size) {
    byte_header_t *head, *split, *tail;

    if (byte_block_needs_split(block, size)) {
        head  = *block;
        tail  = head->next;
        split = (void *) (head + 1) + size;
        head->next  = split;
        split->next = tail;
    }
}

static void *byte_block_allocate(byte_pool_t *pool, byte_header_t **block, size_t size) {
    void *return_ptr = NULL;

    if (byte_block_needs_split(block, size)) {
        byte_block_split(block, size);
        pool->fragments++;
    }

    if (byte_block_get_size(block) >= size) {
        (*block)->owner = pool;
        return_ptr = *block + 1;
        pool->capacity -= (byte_block_get_size(block) + sizeof(void *));
    }

    return return_ptr;
}

static void byte_block_free(byte_pool_t *pool, byte_header_t **block) {
    (*block)->owner = NULL;
    pool->capacity += byte_block_get_size(block) + sizeof(byte_header_t);
}
static byte_header_t *byte_block_get_next(byte_header_t *block) {
    byte_header_t *next = NULL;

    if (block != NULL && byte_block_is_valid(block->next)) {
        next = block->next;
    }

    return next;
}


void byte_pool_init(byte_pool_t *pool, void *memory, size_t size) {
    byte_header_t *header;

    pool->start     = memory;
    pool->search    = memory;
    pool->end       = memory + size - sizeof(byte_header_t);
    pool->fragments = 0;
    pool->capacity      = size - sizeof(byte_header_t) - sizeof(byte_header_t);

    header = pool->start;
    header->owner       = NULL;
    header->next        = pool->end;
    header->next->next  = NULL;
    header->next->owner = pool;
}

void byte_pool_defragment(byte_pool_t *pool) {
    byte_header_t *block, *next;

    if (pool != NULL) {
        block = pool->start;
        while (byte_block_is_valid(block)) {
            if (byte_block_is_free(block)) {
                next = byte_block_get_next(block);
                if (byte_block_is_valid(next) && byte_block_is_free(next)) {
                    byte_block_merge_next(pool, block);
                } else {
                    block = byte_block_get_next(next);
                }
            } else {
                block = byte_block_get_next(block);
            }
        }
    }
}

void *byte_allocate(byte_pool_t *pool, size_t size) {
    void          *return_ptr = NULL;
    byte_header_t *block;
    byte_header_t *next;

    if (byte_pool_is_valid(pool)) {
        block = pool->search;

        while (byte_block_is_valid(block) && return_ptr == NULL) {
            if (byte_block_is_free(block)) {
                if (byte_block_get_size(&block) < size) {
                    next = byte_block_get_next(block);

                    if (byte_block_is_valid(next) && byte_block_is_free(next)) {
                        byte_block_merge_next(pool, block);
                    } else {
                        block = byte_block_get_next(next);
                    }
                } else {
                    return_ptr = byte_block_allocate(pool, &block, size);
                }
            } else {
                block = byte_block_get_next(block);
            }
        }
    }

    return return_ptr;
}


void byte_release(void *memory) {
    byte_header_t *block = get_header_from_memory(memory);

    if (!byte_block_is_free(block)) {
        byte_pool_t *pool = block->owner;
        if(byte_pool_is_valid(pool)) {
            byte_block_free(pool, &block);
        } else {
            ERROR(14, "Byte Pool Pointer Error");
        }
    } else {
        ERROR(14, "Byte Pool Pointer Error");
    }
}

int byte_pool_is_valid(byte_pool_t *pool) {
    return (pool != NULL)
           && (pool->start < pool->end)
           && (pool->capacity <= (pool->end - pool->start));
}