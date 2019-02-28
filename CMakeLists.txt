cmake_minimum_required(VERSION 3.8)
project(cpool C CXX)

set(CMAKE_C_STANDARD 99)

add_library(cpool
        include/block_pool.h
        include/byte_pool.h
        include/segment_pool.h
        source/block_pool.c
        source/byte_pool.c
        source/segment_pool.c)

target_include_directories(cpool PUBLIC include INTERFACE include)

add_subdirectory(extern/googletest)
include_directories(extern/googletest/googletest/include extern/googletest/googlemock/include)

add_executable(test_all
        test/test_block_pool.cpp
        test/test_byte_pool.cpp
        test/test_segment_pool.cpp)

target_link_libraries(test_all PUBLIC PRIVATE cpool gtest gmock gtest_main)