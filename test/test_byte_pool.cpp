//
// Created by Andrew Wade on 2019-01-18.
//
#include <gtest/gtest.h>
#include "byte_pool.h"

extern "C" {
typedef struct byte_header_t byte_header_t;

struct byte_header_t {
    byte_header_t *next;
    byte_pool_t   *owner;
};

byte_header_t *get_header_from_memory(void *memory);
bool byte_block_is_free(byte_header_t *block);
bool byte_block_is_valid(byte_header_t *block);
size_t byte_block_get_size(byte_header_t **block);
void byte_block_merge_next(byte_pool_t *pool, byte_header_t *block);
bool byte_block_needs_split(byte_header_t **block, size_t size);
void *byte_block_allocate(byte_pool_t *pool, byte_header_t **block, size_t size);
void byte_block_split(byte_pool_t *pool, byte_header_t **block, size_t size);
void byte_block_free(byte_pool_t *pool, byte_header_t *block);
byte_header_t *byte_block_get_next(byte_header_t *);
};

class BytePoolTestFixture : public testing::Test {
public:
    BytePoolTestFixture() : pool((byte_pool_t) {NULL, NULL, NULL, 0, 0}),
                            empty((byte_pool_t) {NULL, NULL, NULL, 0, 0}),
                            buffer(),
                            size(256) {

    }

    void SetUp() {

    }

    void PoolInit() {
        byte_pool_init(&pool, buffer, size);
    }

    void PoolDump() {
        byte_header_t *curr = (byte_header_t *) pool.start;

        while (curr != NULL) {
            printf("\n");
            printf("Address: %p\n", (void *) curr);
            printf("   Next: %p\n", (void *) byte_block_get_next(curr));
            printf("   Free: %d\n", byte_block_is_free(curr));
            printf("   Size: %lu\n", byte_block_get_size(&curr));

            curr = byte_block_get_next(curr);
        }
    }

    byte_pool_t       pool;
    const byte_pool_t empty;
    uint8_t           buffer[256];
    const size_t      size;
};

TEST_F(BytePoolTestFixture, private_get_header_from_memory) {
    byte_header_t header;
    EXPECT_EQ(get_header_from_memory(&header + 1), &header);
}

TEST_F(BytePoolTestFixture, private_byte_block_is_free) {
    byte_header_t block = {.owner = NULL};
    EXPECT_FALSE(byte_block_is_free(NULL));
    EXPECT_TRUE(byte_block_is_free(&block));
    block.owner = &pool;
    EXPECT_FALSE(byte_block_is_free(&block));
}

TEST_F(BytePoolTestFixture, private_byte_block_is_valid) {
    byte_header_t block = {.owner = NULL, .next = NULL};
    byte_header_t next;
    EXPECT_FALSE(byte_block_is_valid(NULL));
    EXPECT_FALSE(byte_block_is_valid(&block));
    block.next = &next;
    EXPECT_TRUE(byte_block_is_valid(&block));
}

TEST_F(BytePoolTestFixture, private_byte_block_get_size) {
    byte_header_t block      = {.owner = &pool, .next = NULL};
    byte_header_t *block_ptr = &block;
    byte_header_t *next      = block_ptr + 2;
    EXPECT_EQ(byte_block_get_size(NULL), 0);
    EXPECT_EQ(byte_block_get_size(&block_ptr), 0);
    block.next = next;
    EXPECT_EQ(byte_block_get_size(&block_ptr), sizeof(block));
}

TEST_F(BytePoolTestFixture, private_byte_block_merge_next) {
    byte_header_t block[6] = {
        {.owner = &pool, .next = &block[2]},
        {.owner = NULL, .next = NULL},
        {.owner = NULL, .next = &block[4]},
        {.owner = NULL, .next = NULL},
        {.owner = NULL, .next = &block[5]},
        {.owner = NULL, .next = NULL}};
    byte_header_t *first   = block;
    byte_header_t *second  = &block[2];
    byte_header_t *third   = &block[4];
    byte_header_t *last    = &block[5];
    pool.capacity  = sizeof(byte_header_t) * 3;
    pool.search    = second;
    pool.fragments = 3;
    pool.start     = block;
    pool.end       = block + 6;

    byte_pool_t pool_before_merge = pool;

    byte_block_merge_next(NULL, first);
    EXPECT_EQ(memcmp(&pool_before_merge, &pool, sizeof(pool)), 0);

    byte_block_merge_next(&pool, NULL);
    EXPECT_EQ(memcmp(&pool_before_merge, &pool, sizeof(pool)), 0);

    byte_block_merge_next(&pool, first);
    EXPECT_EQ(pool.fragments, pool_before_merge.fragments - 1);
    EXPECT_EQ(pool.search, first);
    EXPECT_EQ(pool.capacity, pool_before_merge.capacity + sizeof(byte_header_t));
    EXPECT_EQ(first->next, third);
}

TEST_F(BytePoolTestFixture, private_byte_block_needs_split) {
    byte_header_t block      = {NULL, NULL};
    byte_header_t *block_ptr = &block;

    // null block should be false
    EXPECT_FALSE(byte_block_needs_split(NULL, 16));

    // zero size should be false
    EXPECT_FALSE(byte_block_needs_split(&block_ptr, 0));

    // bad block should be false
    EXPECT_FALSE(byte_block_needs_split(&block_ptr, 16));

    // good block should be true if remaining size >= BYTE_BLOCK_MIN
    block.next = block_ptr + 3;
    EXPECT_TRUE(byte_block_needs_split(&block_ptr, sizeof(block)));

    // good block should be false if remaining size <BYTE_BLOCK_MIN
    block.next = block_ptr + 2;
    EXPECT_FALSE(byte_block_needs_split(&block_ptr, sizeof(block)));
}

TEST_F(BytePoolTestFixture, private_byte_block_split) {
    byte_header_t block[6] = {
        {.owner = &pool, .next = &block[4]},
        {.owner = NULL, .next = NULL},
        {.owner = NULL, .next = NULL},
        {.owner = NULL, .next = NULL},
        {.owner = NULL, .next = &block[5]},
        {.owner = NULL, .next = NULL}};
    byte_header_t *first   = block;
    byte_header_t *second  = &block[2];
    byte_header_t *third   = &block[4];
    byte_header_t *last    = &block[5];
    pool.capacity  = sizeof(byte_header_t) * 3;
    pool.search    = second;
    pool.fragments = 3;
    pool.start     = block;
    pool.end       = block + 6;

    byte_pool_t pool_before_split = pool;

    // should ignore null pool
    byte_block_split(NULL, &first, sizeof(byte_header_t));
    EXPECT_EQ(memcmp(&pool_before_split, &pool, sizeof(pool)), 0);

    // should ignore null block
    byte_block_split(&pool, NULL, sizeof(byte_header_t));
    EXPECT_EQ(memcmp(&pool_before_split, &pool, sizeof(pool)), 0);

    // should ignore zero size
    byte_block_split(&pool, &first, 0);
    EXPECT_EQ(memcmp(&pool_before_split, &pool, sizeof(pool)), 0);

    // should split to second pointer
    byte_block_split(&pool, &first, sizeof(byte_header_t));
    EXPECT_EQ(first->next, second);
    EXPECT_EQ(second->next, third);
    EXPECT_EQ(pool.fragments, pool_before_split.fragments + 1);
    EXPECT_EQ(pool.capacity, pool_before_split.capacity - sizeof(byte_header_t));
    EXPECT_EQ(pool.search, pool_before_split.search);
}

TEST_F(BytePoolTestFixture, private_byte_block_allocate) {
    PoolInit();
    byte_pool_t   pool_before_allocate  = pool;
    byte_header_t block_before_allocate = *(byte_header_t *) pool.start;
    byte_header_t *block                = (byte_header_t *) pool.start;

    // should ignore null pool
    byte_block_allocate(NULL, &block, sizeof(byte_header_t));
    EXPECT_EQ(memcmp(&pool_before_allocate, &pool, sizeof(pool)), 0);
    EXPECT_EQ(memcmp(&block_before_allocate, pool.start, sizeof(block_before_allocate)), 0);

    // should ignore null block
    byte_block_allocate(&pool, NULL, sizeof(byte_header_t));
    EXPECT_EQ(memcmp(&pool_before_allocate, &pool, sizeof(pool)), 0);
    EXPECT_EQ(memcmp(&block_before_allocate, pool.start, sizeof(block_before_allocate)), 0);

    // should ignore size zero
    byte_block_allocate(&pool, &block, 0);
    EXPECT_EQ(memcmp(&pool_before_allocate, &pool, sizeof(pool)), 0);
    EXPECT_EQ(memcmp(&block_before_allocate, pool.start, sizeof(block_before_allocate)), 0);

    // should reduce capacity and set owner and move search pointer
    byte_block_allocate(&pool, &block, sizeof(byte_header_t));
    EXPECT_NE(memcmp(&block_before_allocate, pool.start, sizeof(block_before_allocate)), 0);
    EXPECT_EQ(pool.capacity, pool_before_allocate.capacity - sizeof(byte_header_t) * 2);
    EXPECT_EQ(block->owner, &pool);
    EXPECT_EQ(pool.search, &buffer[sizeof(byte_header_t) * 2]);
}

TEST_F(BytePoolTestFixture, byte_block_free) {
    PoolInit();
    void          *data             = byte_allocate(&pool, 16);
    byte_pool_t   pool_before_free  = pool;
    byte_header_t *block            = (byte_header_t *) pool.start;
    byte_header_t block_before_free = *block;

    // should ignore NULL pool
    byte_block_free(NULL, block);
    EXPECT_EQ(memcmp(&pool_before_free, &pool, sizeof(pool)), 0);
    EXPECT_EQ(memcmp(block, &block_before_free, sizeof(block_before_free)), 0);

    // should ignore NULL block
    byte_block_free(&pool, NULL);
    EXPECT_EQ(memcmp(&pool_before_free, &pool, sizeof(pool)), 0);
    EXPECT_EQ(memcmp(block, &block_before_free, sizeof(block_before_free)), 0);

    // should release owner and increase capacity
    byte_block_free(&pool, block);
    EXPECT_EQ(block->owner, nullptr);
    EXPECT_EQ(pool.capacity, pool_before_free.capacity + 16 + sizeof(byte_header_t));
}

TEST_F(BytePoolTestFixture, byte_block_get_next) {
    byte_header_t invalid_block = {NULL, NULL};
    byte_header_t *block_ptr;
    EXPECT_EQ(byte_block_get_next(NULL), nullptr);
    EXPECT_EQ(byte_block_get_next(&invalid_block), nullptr);

    // return null if block is the last block in pool list
    PoolInit();
    EXPECT_EQ(byte_block_get_next((byte_header_t *) pool.start), nullptr);

    // return next if next block is valid
    byte_allocate(&pool, 16);
    block_ptr = (byte_header_t*)pool.start;
    EXPECT_EQ(byte_block_get_next(block_ptr), ((byte_header_t *) pool.start)->next);
    // return null when block is end of pool
    EXPECT_EQ(byte_block_get_next(block_ptr->next), nullptr);
}

TEST_F(BytePoolTestFixture, init_ignores_bad_inputs) {
    byte_pool_init(NULL, buffer, size);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);

    byte_pool_init(&pool, NULL, size);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);

    byte_pool_init(&pool, buffer, 0);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);

    byte_pool_init(&pool, buffer, BYTE_BLOCK_MIN - 1);
    EXPECT_EQ(memcmp(&pool, &empty, sizeof(pool)), 0);
}

TEST_F(BytePoolTestFixture, init_configures_correctly) {
    PoolInit();
    EXPECT_EQ(pool.fragments, 1);
    EXPECT_EQ(pool.start, buffer);
    EXPECT_EQ(pool.end, &buffer[size] - sizeof(byte_header_t));
    EXPECT_EQ(pool.capacity, size - (sizeof(byte_header_t) * 2));
    EXPECT_EQ(pool.search, pool.start);
}

TEST_F(BytePoolTestFixture, allocate_ignores_bad_inputs) {
    PoolInit();
    void *result = byte_allocate(NULL, 8);
    EXPECT_EQ(result, nullptr);
    result = byte_allocate(&pool, 0);
    EXPECT_EQ(result, nullptr);
}

TEST_F(BytePoolTestFixture, allocate_moves_search_pointer) {
    PoolInit();
    void *expected_search_pointer_after_allocate = (uint8_t *) pool.search + 32 + sizeof(byte_header_t);
    byte_allocate(&pool, 32);
    EXPECT_EQ(expected_search_pointer_after_allocate, pool.search);
}

TEST_F(BytePoolTestFixture, allocate_returns_new_pointer_until_empty) {
    PoolInit();

    void   *allocations[size];
    size_t expected_capacity_after_allocate;

    for (int i = 0; pool.capacity > 16; i++) {
        expected_capacity_after_allocate = pool.capacity - 16 - sizeof(byte_header_t);
        allocations[i] = byte_allocate(&pool, 16);
        EXPECT_EQ(expected_capacity_after_allocate, pool.capacity);
        ASSERT_NE(allocations[i], nullptr);
        for (int j     = 0; j < i; j++) {
            EXPECT_NE(allocations[i], allocations[j]);
        }
    }
    EXPECT_EQ(byte_allocate(&pool, 16), nullptr);
}

TEST_F(BytePoolTestFixture, is_valid_catches_bad_pool_config) {
    // start cannot be null
    PoolInit();
    pool.start = NULL;
    EXPECT_FALSE(byte_pool_is_valid(&pool));

    // end cannot be null
    PoolInit();
    pool.end = NULL;
    EXPECT_FALSE(byte_pool_is_valid(&pool));

    // end cannot equal start
    PoolInit();
    pool.end = pool.start;
    EXPECT_FALSE(byte_pool_is_valid(&pool));

    // end cannot be less than start
    PoolInit();
    pool.end = (int *) pool.start - 1;

    // search cannot be null if capacity is not zero
    PoolInit();
    pool.search = NULL;
    EXPECT_FALSE(byte_pool_is_valid(&pool));

    // search must be null if capacity is zero
    PoolInit();
    pool.search   = NULL;
    pool.capacity = 0;
    EXPECT_TRUE(byte_pool_is_valid(&pool));

    // capacity cannot be greater or equal to end - start
    PoolInit();
    pool.capacity = (char *) pool.end - (char *) pool.start;
    EXPECT_FALSE(byte_pool_is_valid(&pool));

    // fragments cannot be zero
    PoolInit();
    pool.fragments = 0;
    EXPECT_FALSE(byte_pool_is_valid(&pool));
}

TEST_F(BytePoolTestFixture, byte_size_returns_zero_for_invalid_inputs) {
    PoolInit();

    // null should return 0
    EXPECT_EQ(byte_size(NULL), 0);

    // free blocks should return zero
    EXPECT_EQ(byte_size(((byte_header_t*)pool.start)+1), 0);
}

TEST_F(BytePoolTestFixture, byte_size_returns_correct_size_for_allocated_memory) {
    PoolInit();
    // non-free blocks should return correct size
    byte_allocate(&pool, 32);
    EXPECT_EQ(byte_size(((byte_header_t*)pool.start)+1), 32);
}