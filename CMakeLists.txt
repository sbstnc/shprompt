cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(git_prompt_c)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/stb)
add_subdirectory(deps/libchalk)
add_subdirectory(deps/libsds)
add_subdirectory(deps/libgit2)
add_subdirectory(libshprompt)
add_subdirectory(shprompt)

