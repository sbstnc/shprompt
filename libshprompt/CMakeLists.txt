cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(libshprompt
        VERSION 1.0
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_library(libshprompt STATIC
            libshprompt/config.h
            libshprompt/modules/directory.c libshprompt/modules/directory.h
            libshprompt/modules/git.c libshprompt/modules/git.h
            libshprompt/modules/character.c libshprompt/modules/character.h
            libshprompt/modules/status.c libshprompt/modules/status.h
            libshprompt/module.c libshprompt/module.h
            libshprompt/context.h
            libshprompt/segment.h libshprompt/segment.c libshprompt/config.c)
target_include_directories(libshprompt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/deps/libgit2/include)
target_link_libraries(libshprompt libsds stb git2 libchalk)