cmake_minimum_required(VERSION 3.10)

set(CXX_FLAGS
    -g
    -Wall
    -Werror
    -std=c++17
    )

add_subdirectory(network)
add_subdirectory(test)

