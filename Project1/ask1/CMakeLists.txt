cmake_minimum_required(VERSION 3.29)
project(ask1 C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(ask1
    src/pipes.c
    src/pipes.h)
