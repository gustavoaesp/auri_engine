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

        #vma
        third_party/VulkanMemoryAllocator/VmaUsage.cpp
    )

    set (RENDER_BACKEND_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/VulkanMemoryAllocator
    )

    #add_compile_definitions(VMA_IMPLEMENTATION)
    add_compile_definitions(VULKAN_ENABLED)
endif()

add_library(pengine
    ${RENDER_BACKEND_SRC}
)
target_link_libraries(pengine vulkan)
target_link_libraries(pengine glfw)
target_include_directories(pengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(pengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/render)
target_include_directories(pengine PUBLIC ${RENDER_BACKEND_INCLUDE_DIRS})