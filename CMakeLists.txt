cmake_minimum_required(VERSION 3.8)
project(sockets)

set(CMAKE_CXX_STANDARD 20)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

link_directories("/opt/homebrew/lib")
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(test_tree
    tests/tree_test.cpp
    include/nvwa/debug_new.cpp)
target_compile_options(test_tree PUBLIC "-g" "-O3")
target_link_libraries(test_tree PUBLIC tcmalloc c++)

add_executable(server_test
    src/server.cpp
    include/nvwa/debug_new.cpp)
target_compile_options(server_test PUBLIC "-g" "-pthread" "-mtune=native" "-O3")
target_link_libraries(server_test "${CMAKE_THREAD_LIBS_INIT}" ZLIB::ZLIB c++)
add_executable(client_test
    src/client.cpp
    include/nvwa/debug_new.cpp)
target_compile_options(client_test PUBLIC "-pthread" "-mtune=native" "-O3")
target_link_libraries(client_test "${CMAKE_THREAD_LIBS_INIT}" ZLIB::ZLIB c++)

add_executable(buffer_test
    tests/buffer_test.cpp)
target_link_libraries(buffer_test ZLIB::ZLIB)
