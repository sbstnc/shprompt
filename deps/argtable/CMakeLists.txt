cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(argtable
        VERSION 3.1.5
        LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_library(argtable argtable/argtable3.h argtable/argtable3.c)
target_include_directories(argtable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})