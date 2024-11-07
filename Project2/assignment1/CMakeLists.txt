cmake_minimum_required(VERSION 3.29)
project(assignment_1 C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(assignment_1
    src/mysem.c
    src/mysem.h
        test/test_mysem.c)
