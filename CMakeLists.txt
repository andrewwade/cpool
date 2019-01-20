cmake_minimum_required(VERSION 3.13)
project(malloc_override_test C)

set(CMAKE_C_STANDARD 99)

add_library(memorypool
        include/block_pool.h
        include/byte_pool.h
        source/block_pool.c
        source/byte_pool.c
        )

target_include_directories(memorypool PUBLIC PRIVATE include)

add_executable(test_main
        test/test_main.c
        test/test_block_pool.c
        test/test_byte_pool.c test/test_block_pool.h test/test_byte_pool.h)

target_include_directories(test_main PUBLIC PRIVATE source PUBLIC PRIVATE include)
target_link_libraries(test_main PUBLIC PRIVATE memorypool)