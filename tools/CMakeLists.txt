cmake_minimum_required(VERSION 3.0)

add_executable(img_converter img_converter.cpp)
target_include_directories(img_converter PUBLIC ${CMAKE_SOURCE_DIR}/runtime)
    

#add_executable(fbx_converter
#    fbx_converter.cpp
#    ofbx/miniz.c
#    ofbx/ofbx.cpp
#)
#target_include_directories(
#    fbx_converter
#    PUBLIC
#    ${CMAKE_SOURCE_DIR}/runtime
#    ${CMAKE_CURRENT_SOURCE_DIR}
#)
#target_link_libraries(
#    fbx_converter
#    pengine
#)