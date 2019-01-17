cmake_minimum_required(VERSION 3.13)
project(malloc_override_test C)

set(CMAKE_C_STANDARD 99)

add_library(memorypool
        source/block_pool.c
        source/block_pool.h
        include/block_pool.h
        source/byte_pool.c
        source/byte_pool.h
        include/block_pool.h)
target_include_directories(memorypool INTERFACE include)

add_executable(test_main
        test/test_main.c
        source/block_pool.c
        source/block_pool.h
        include/block_pool.h
        source/byte_pool.c
        source/byte_pool.h
        include/block_pool.h)