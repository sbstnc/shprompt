cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(shprompt
        VERSION 1.0
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(shprompt shprompt/shprompt.c)
target_include_directories(shprompt PRIVATE ${CMAKE_SOURCE_DIR}/deps/flag ${CMAKE_SOURCE_DIR}/deps/colorize)
target_link_libraries(shprompt libshprompt argtable)