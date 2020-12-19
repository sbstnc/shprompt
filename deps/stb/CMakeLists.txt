cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(libsds
        VERSION 2.0.0
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})