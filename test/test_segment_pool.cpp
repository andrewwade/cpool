//
// Created by Andrew Wade on 2019-01-20.
//

#include <gtest/gtest.h>
#include <segment_pool.h>

class SegmentPoolTestFixture : public testing::Test {
public:

    void SetUp() {

    }

    void TearDown() {

    }
};

TEST_F(SegmentPoolTestFixture, standard_usage) {
    int buffer[512];

    segment_pool_t buffer_pool;
    segment_pool_t *pool = &buffer_pool;

    segment_pool_init(pool, sizeof(int), buffer, buffer + 512);

    int *v1 = (int*) segment_allocate(pool);

    int *v4 = (int*) segment_allocate_size(pool, sizeof(int) * 4);

    segment_release_size(pool, v4, sizeof(int) * 4);
}

TEST_F(SegmentPoolTestFixture, init_should_ignore_bad_inputs) {

}

