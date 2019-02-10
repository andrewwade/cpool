//
// Created by Andrew Wade on 2019-01-18.
//
#include "gtest/gtest.h"
#include "block_pool.h"
#include "memory.h"

typedef union block_header_t block_header_t;
union block_header_t {
    block_header_t *next;
    block_pool_t *owner;
};

class BlockPoolTestFixture : public testing::Test {
public:

    BlockPoolTestFixture() = default;

    void SetUp() {
        memcpy(&pool, &empty, sizeof(pool));
        for(int i = 0; i < 16; i++) {
            memcpy(&buffer[i], &empty, sizeof(empty));
        }
    }

    void InitPool() {
        block_pool_init(&pool, sizeof(pool), buffer, buffer + size);
    }

    const int size = 16;
    block_pool_t pool;
    block_pool_t buffer[16];


    const block_pool_t empty = {
        .alignment = 0,
        .start = NULL,
        .end = NULL,
        .search = NULL,
        .capacity = 0,
        .available = 0,
    };

};

TEST_F(BlockPoolTestFixture, init_handles_invalid_arguments) {
    // dont allow null pool
    block_pool_init(NULL, 4, buffer, buffer + size);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);

    // don't allow 0 alignment
    block_pool_init(&pool, 0, buffer, buffer + size);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);

    // don't allow null start
    block_pool_init(&pool, sizeof(int), NULL, buffer + size);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);

    // don't allow null end
    block_pool_init(&pool, sizeof(int), buffer, NULL);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);
}

TEST_F(BlockPoolTestFixture, init_sets_correct_values) {
    block_pool_init(&pool, sizeof(pool), buffer, buffer+size);
    block_header_t *block = static_cast<block_header_t *>(pool.start);

    EXPECT_EQ(pool.alignment, sizeof(pool));
    EXPECT_EQ(pool.capacity, size * sizeof(pool)/(pool.alignment+sizeof(void*)));
    EXPECT_LT(pool.capacity, size);
    EXPECT_EQ(pool.capacity, pool.available);
    EXPECT_EQ(pool.start, buffer);
    EXPECT_EQ(pool.end, &buffer[size]);
    EXPECT_EQ(pool.search, pool.start);

    for(int i = 0; i < pool.capacity; i++){
        EXPECT_EQ(block->next, (void*)(((char*)(block+1)+pool.alignment)));
        block = block->next;
    }
}

TEST_F(BlockPoolTestFixture, allocate_shifts_search_pointer) {
    InitPool();
    block_header_t *next_expected_search = ((block_header_t *)pool.start)->next;
    block_allocate(&pool);
    EXPECT_EQ(pool.search, next_expected_search);
}

TEST_F(BlockPoolTestFixture, allocate_decreases_available) {
    InitPool();
    block_pool_t *new_pool = (block_pool_t *)block_allocate(&pool);
    EXPECT_EQ(pool.available, pool.capacity - 1);
}

TEST_F(BlockPoolTestFixture, allocate_returns_new_block_until_empty) {
    InitPool();
    void *allocations[pool.available];
    for (int i = 0; i < pool.capacity; i++){
        allocations[i] = (block_pool_t *)block_allocate(&pool);
        for(int j = 0; j < i; j++) {
            // check for duplicate pointers
            EXPECT_NE(allocations[i], allocations[j]);
        }
    }
    void *empty_allocation = block_allocate(&pool);
    EXPECT_EQ(empty_allocation, nullptr);
}

TEST_F(BlockPoolTestFixture, release_ignores_null) {
    InitPool();
    block_pool_t pool_before_release = pool;
    block_release(NULL);
    EXPECT_EQ(memcmp(&pool_before_release, &pool, sizeof(pool)), 0);
}

TEST_F(BlockPoolTestFixture, release_ignores_memory_not_from_block_pool) {
    InitPool();
    block_pool_t pool_before_release = pool;
    block_header_t header = {.owner = (block_pool_t*)&empty};
    block_release(&header+1);
    EXPECT_EQ(memcmp(&pool_before_release, &pool, sizeof(pool)), 0);
}

TEST_F(BlockPoolTestFixture, release_moves_search_pointer_to_released) {
    InitPool();
    void *block = block_allocate(&pool);
    EXPECT_NE(block, (char*)pool.search+sizeof(void*));
    block_release(block);
    EXPECT_EQ(block, (char*)pool.search+sizeof(void*));
}

TEST_F(BlockPoolTestFixture, release_increases_available) {
    InitPool();
    size_t expected_available_after_release = pool.available;
    void *block = block_allocate(&pool);
    EXPECT_NE(expected_available_after_release, pool.available);
    block_release(block);
    EXPECT_EQ(expected_available_after_release, pool.available);
}
