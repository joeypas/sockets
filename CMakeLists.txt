cmake_minimum_required(VERSION 3.8)
project(sockets)

set(CMAKE_CXX_STANDARD 23)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include"
    "/Users/josephliotta/dev/cppcoro/include")

link_directories("/Users/josephliotta/dev/cppcoro/build/darwin_x64_clang16.0.2_optimised/lib")

add_executable(test_tree 
    tests/tree_test.cpp)

target_link_libraries(test_tree cppcoro)


