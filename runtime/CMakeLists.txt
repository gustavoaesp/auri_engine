cmake_minimum_required(VERSION 3.0)

option(BUILD_VULKAN "Build with Vulkan" ON)

if (BUILD_VULKAN)
    set (RENDER_BACKEND_SRC
        render/backend_vulkan/backend.cpp
        render/backend_vulkan/format_lookup.cpp
        render/backend_vulkan/instance.cpp
        render/backend_vulkan/primitives/vk_buffer.cpp
        render/backend_vulkan/primitives/vk_cmd_buffer.cpp
        render/backend_vulkan/primitives/vk_cmd_pool.cpp
        render/backend_vulkan/primitives/vk_descriptors.cpp
        render/backend_vulkan/primitives/vk_framebuffer.cpp
        render/backend_vulkan/primitives/vk_pipeline.cpp
        render/backend_vulkan/primitives/vk_render_pass.cpp
        render/backend_vulkan/primitives/vk_sampler.cpp
        render/backend_vulkan/primitives/vk_shader.cpp
        render/backend_vulkan/primitives/vk_texture.cpp
        render/backend_vulkan/synchronization/vk_semaphore.cpp
        render/backend_vulkan/synchronization/vk_fence.cpp
        render/backend_vulkan/valloc.cpp
        render/backend_vulkan/vertex_input_desc.cpp
        render/backend_vulkan/vk_device.cpp
        render/backend_vulkan/vk_swapchain.cpp
        render/backend_vulkan/transfer_buffers.cpp

        #vma
        third_party/VulkanMemoryAllocator/VmaUsage.cpp

        #imgui
        third_party/imgui/backends/imgui_impl_glfw.cpp
        third_party/imgui/backends/imgui_impl_vulkan.cpp
    )

    set (RENDER_BACKEND_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/VulkanMemoryAllocator
    )

    #add_compile_definitions(VMA_IMPLEMENTATION)
    add_compile_definitions(VULKAN_ENABLED)
endif()

set(RENDER_SRC
    render/transform/matrix.cpp
    render/transform/vector.cpp
    render/mesh.cpp
    render/managers/mesh_manager.cpp
    render/managers/texture_manager.cpp
    render/scene/scene_mesh.cpp
    render/mesh.cpp
    render/renderer.cpp
    render/stages/render_stage_geometry.cpp

    # ImGui
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
)

set(RENDER_INCLUDES
    third_party/imgui
    third_party/imgui/backends
)

add_library(pengine
    ${RENDER_BACKEND_SRC}
    ${RENDER_SRC}
)
target_link_libraries(pengine vulkan)
target_link_libraries(pengine glfw)
target_include_directories(pengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/render)
target_include_directories(pengine PUBLIC ${RENDER_BACKEND_INCLUDE_DIRS})
target_include_directories(pengine PUBLIC ${RENDER_INCLUDES})