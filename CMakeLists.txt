cmake_minimum_required(VERSION 3.8)
project(sockets)

set(CMAKE_CXX_STANDARD 20)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(test_tree
    tests/tree_test.cpp)
target_compile_options(test_tree PUBLIC "-mtune=native" "-O3")

add_executable(server_test
    src/server.cpp)
target_compile_options(server_test PUBLIC "-pthread" "-g")
target_link_libraries(server_test "${CMAKE_THREAD_LIBS_INIT}" ZLIB::ZLIB)
add_executable(client_test
    src/client.cpp)
target_compile_options(client_test PUBLIC "-pthread" "-g")
target_link_libraries(client_test "${CMAKE_THREAD_LIBS_INIT}" ZLIB::ZLIB)

add_executable(buffer_test
    tests/buffer_test.cpp)
target_link_libraries(buffer_test ZLIB::ZLIB)