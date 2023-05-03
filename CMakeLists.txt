cmake_minimum_required(VERSION 3.8)
project(sockets)

set(CMAKE_CXX_STANDARD 20)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")
find_package(Threads REQUIRED)

add_executable(test_tree
    tests/tree_test.cpp)
target_link_libraries(test_tree PRIVATE cppcoro)

add_executable(server_test
    src/server.cpp)
target_compile_options(server_test PUBLIC "-pthread")
target_link_libraries(server_test "${CMAKE_THREAD_LIBS_INIT}")
add_executable(client_test
    src/client.cpp)
target_compile_options(client_test PUBLIC "-pthread")
target_link_libraries(client_test "${CMAKE_THREAD_LIBS_INIT}")