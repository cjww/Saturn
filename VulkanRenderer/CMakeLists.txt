add_library(VulkanRenderer STATIC)

file(GLOB_RECURSE RENDERER_SRC
        src/*.cpp
)

file(GLOB_RECURSE RENDERER_HEADERS
        include/*.h
        include/*.hpp
)

target_sources(VulkanRenderer PRIVATE ${RENDERER_SRC} ${RENDERER_HEADERS})

target_precompile_headers(VulkanRenderer PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h>"
)

target_include_directories(VulkanRenderer
        PUBLIC
        include
)

target_link_libraries(VulkanRenderer
        PUBLIC
        shaderc
        spirv-cross-core
        glfw
        Vulkan::Vulkan
)