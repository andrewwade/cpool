//
// Created by Andrew Wade on 2019-01-12.
//

#include "byte_pool.h"
#include <stdbool.h>

typedef struct byte_header_t byte_header_t;

struct byte_header_t {
    byte_header_t *next;
    byte_pool_t   *owner;
};

byte_header_t *get_header_from_memory(void *memory);

bool byte_block_is_free(byte_header_t *block);

bool byte_block_is_valid(byte_header_t *block);

size_t byte_block_get_size(byte_header_t **block);

bool byte_block_needs_merge(byte_header_t *block) {
    return byte_block_is_valid(block)
           && byte_block_is_valid(block->next)
           && byte_block_is_free(block->next);
}

void byte_block_merge_next(byte_pool_t *pool, byte_header_t *block);

bool byte_block_needs_split(byte_header_t **block, size_t size);

void *byte_block_allocate(byte_pool_t *pool, byte_header_t **block, size_t size);

void byte_block_split(byte_pool_t *pool, byte_header_t **block, size_t size);

void byte_block_free(byte_pool_t *pool, byte_header_t *block);

byte_header_t *byte_block_get_next(byte_header_t *);

void byte_pool_init(byte_pool_t *pool, void *memory, size_t size) {
    byte_header_t *header;

    if (pool != NULL && memory != NULL && size > BYTE_BLOCK_MIN) {
        pool->start     = memory;
        pool->search    = memory;
        pool->end       = memory + size - sizeof(byte_header_t);
        pool->fragments = 1;
        pool->capacity  = size - sizeof(byte_header_t) - sizeof(byte_header_t);

        header = pool->start;
        header->owner       = NULL;
        header->next        = pool->end;
        header->next->next  = NULL;
        header->next->owner = pool;
    }
}

void byte_pool_defragment(byte_pool_t *pool) {
    byte_header_t *block, *next;

    if (pool != NULL) {
        block = pool->start;
        while (byte_block_is_valid(block)) {
            if (byte_block_is_free(block)) {
                if (byte_block_needs_merge(block)) {
                    byte_block_merge_next(pool, block);
                } else {
                    // skip next because it is not free
                    next = byte_block_get_next(block);
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

    if (byte_pool_is_valid(pool) && size > 0) {
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
        if (byte_pool_is_valid(pool)) {
            byte_block_free(pool, block);
        }
    }
}

size_t byte_size(void *memory) {
    if(memory != NULL) {
        byte_header_t *block = get_header_from_memory(memory);
        if (!byte_block_is_free(block)) {
            byte_pool_t *pool = block->owner;
            if (byte_pool_is_valid(pool)) {
                return byte_block_get_size(&block);
            }
        }
    }
    return 0;
}

int byte_pool_is_valid(byte_pool_t *pool) {
    return (pool != NULL)
           && (pool->start != NULL)
           && (pool->end != NULL)
           && (pool->search != NULL || pool->capacity == 0)
           && (pool->start < pool->end)
           && (pool->capacity < (pool->end - pool->start))
           && (pool->fragments > 0)
           && (pool->fragments < (pool->end - pool->start) / sizeof(byte_header_t));
}

byte_header_t *get_header_from_memory(void *memory) {
    return ((byte_header_t *) memory) - 1;
}

bool byte_block_is_free(byte_header_t *block) {
    return (block != NULL) && block->owner == NULL;
}

bool byte_block_is_valid(byte_header_t *block) {
    return (block != NULL) && (block->next != NULL);
}

size_t byte_block_get_size(byte_header_t **block) {
    size_t size = 0;
    if (block != NULL && byte_block_is_valid(*block)) {
        size = ((void *) (*block)->next - (void *) (*block + 1));
    }
    return size;
}

void byte_block_merge_next(byte_pool_t *pool, byte_header_t *block) {
    if (pool != NULL && block != NULL) {
        byte_header_t *next = block->next;
        block->next = next->next;
        pool->fragments--;
        pool->capacity += sizeof(byte_header_t);
        if (pool->search == next) {
            pool->search = block;

        }
    }
}

bool byte_block_needs_split(byte_header_t **block, size_t size) {
    size_t block_size = byte_block_get_size(block);
    if (block == NULL || size == 0 || block_size == 0) {
        return false;
    } else {
        return (block_size - size >= BYTE_BLOCK_MIN);
    }
}

void byte_block_split(byte_pool_t *pool, byte_header_t **block, size_t size) {
    byte_header_t *head, *split, *tail;
    if (pool != NULL && block != NULL && size > 0) {
        if (byte_block_needs_split(block, size)) {
            head  = *block;
            tail  = head->next;
            split = (void *) (head + 1) + size;
            head->next  = split;
            split->next = tail;
            pool->fragments++;
            pool->capacity -= sizeof(byte_header_t);
        }
    }
}

void *byte_block_allocate(byte_pool_t *pool, byte_header_t **block, size_t size) {
    void *return_ptr = NULL;

    if (pool != NULL && block != NULL && size > 0) {
        if (byte_block_needs_split(block, size)) {
            byte_block_split(pool, block, size);
        }

        if (byte_block_get_size(block) >= size) {
            (*block)->owner = pool;
            return_ptr = *block + 1;
            pool->capacity -= byte_block_get_size(block);
            if ((*block) == pool->search) {
                pool->search = (*block)->next;
            }
        }
    }

    return return_ptr;
}

void byte_block_free(byte_pool_t *pool, byte_header_t *block) {
    if(byte_pool_is_valid(pool) && byte_block_is_valid(block)) {
        block->owner = NULL;
        pool->capacity += byte_block_get_size(&block);
        if(byte_block_needs_merge(block)) {
            byte_block_merge_next(pool, block);
        }
    }
}

byte_header_t *byte_block_get_next(byte_header_t *block) {
    byte_header_t *next = NULL;

    if (block != NULL && byte_block_is_valid(block->next)) {
        next = block->next;
    }

    return next;
}
