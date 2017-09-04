cmake_minimum_required(VERSION 2.8.3)
project(vdb_example_cmake)

include(FindPkgConfig)
pkg_search_module(SDL2 sdl2)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(main main.cpp)
target_link_libraries(main ${SDL2_LIBRARIES} GL)


