cmake_minimum_required(VERSION 3.13)
project(memorypool C CXX)

set(CMAKE_C_STANDARD 99)

add_library(memorypool
        include/block_pool.h
        include/byte_pool.h
        include/chunk_pool.h
        source/block_pool.c
        source/byte_pool.c
        source/chunk_pool.c)

target_include_directories(memorypool PUBLIC include INTERFACE include)

add_subdirectory(extern/googletest)
include_directories(extern/googletest/googletest/include extern/googletest/googlemock/include)

add_executable(test_all
        test/test_block_pool.cpp
        test/test_byte_pool.cpp
        test/test_memory_pool.cpp)

target_link_libraries(test_all PUBLIC PRIVATE memorypool gtest gmock gtest_main)