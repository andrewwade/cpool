#include <stdio.h>
#include <stdbool.h>

#include "test_block_pool.h"
#include "test_byte_pool.h"
#include "test_memory_pool.h"


int main() {
    test_block_pool_run();
    test_byte_pool_run();
    test_memory_pool_run();
    return 0;
}