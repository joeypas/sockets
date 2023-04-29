cmake_minimum_required(VERSION 3.8)
project(sockets)

set(CMAKE_CXX_STANDARD 20)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_executable(test_tree
    tests/tree_test.cpp)
target_link_libraries(test_tree PRIVATE cppcoro)



