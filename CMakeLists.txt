cmake_minimum_required(VERSION 3.0)
project(engine)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(runtime)
add_subdirectory(samples)
add_subdirectory(tools)