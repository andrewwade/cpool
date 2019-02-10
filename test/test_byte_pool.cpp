//
// Created by Andrew Wade on 2019-01-18.
//
#include "gtest/gtest.h"
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
    void byte_block_merge_next(byte_pool_t *pool, byte_header_t *block)
    int byte_block_needs_split(byte_header_t **block, size_t size);
    void *byte_block_allocate(byte_pool_t *pool, byte_header_t **block, size_t size);
    void byte_block_split(byte_header_t **block, size_t size);
    void byte_block_free(byte_pool_t *pool, byte_header_t **block);
    byte_header_t *byte_block_get_next(byte_header_t *);
};

class BytePoolTestFixture : public testing::Test {
public:
    BytePoolTestFixture() {
        pool = empty;
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
    const byte_pool_t empty = {
        .start = NULL,
        .search = NULL,
        .end = NULL,
        .capacity = 0,
        .fragments = 0
    };
    uint8_t           buffer[256];
    const size_t      size  = 256;
};

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
