cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE_Debug)

set(CXX_FLAGS
    -g
    -Wall
    -Werror
    -std=c++17
    )

add_subdirectory(network)
add_subdirectory(test)

