cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(libchalk
        VERSION 0.0.1
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(libchalk INTERFACE)
target_include_directories(libchalk INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})