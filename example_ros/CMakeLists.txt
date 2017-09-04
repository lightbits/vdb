cmake_minimum_required(VERSION 2.8.3)
project(example_vdb)
find_package(catkin REQUIRED roscpp std_msgs)
catkin_package(CATKIN_DEPENDS message_runtime)

include(FindPkgConfig)
pkg_search_module(SDL2 sdl2)
include_directories(${SDL2_INCLUDE_DIRS})
add_executable(gui gui.cpp)
target_link_libraries(gui ${catkin_LIBRARIES} ${SDL2_LIBRARIES} GL)

add_executable(app app.cpp)
target_link_libraries(app ${catkin_LIBRARIES})

