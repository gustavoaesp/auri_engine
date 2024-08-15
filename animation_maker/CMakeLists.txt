cmake_minimum_required(VERSION 3.0)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include (compile_glsl)

add_executable(animation_maker
    main.cpp
    game_mode.cpp
)
target_link_libraries(animation_maker pengine)
target_link_libraries(animation_maker assimp)

set (GLSL_SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/light.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/dirlight.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ambientlight.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vert.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main_skinned.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/frag.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/presentation.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/presentation.frag
)

target_glsl_shaders(animation_maker PUBLIC ${GLSL_SHADERS})