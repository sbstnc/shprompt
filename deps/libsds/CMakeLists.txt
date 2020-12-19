cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(libsds
        VERSION 2.0.0
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(libsds STATIC libsds/sds.c libsds/sds.h libsds/sdsalloc.h)
target_include_directories(libsds PUBLIC libsds ${CMAKE_CURRENT_SOURCE_DIR})