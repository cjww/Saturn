set(PROJECT_NAME VulkanRenderer)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "include/ApiBuildOptions.h"
    "include/DescriptorSetStructs.h"
    "include/DeviceMemoryStats.hpp"
    "include/Format.hpp"
    "include/FormatFlags.hpp"
    "include/Image.hpp"
    "include/InputEnums.hpp"
    "include/internal/CommandPool.hpp"
    "include/internal/debugFunctions.hpp"
    "include/internal/DescriptorSet.hpp"
    "include/internal/DeviceMemoryManager.hpp"
    "include/internal/FramebufferSet.hpp"
    "include/internal/Pipeline.hpp"
    "include/internal/RenderProgram.hpp"
    "include/internal/Swapchain.hpp"
    "include/internal/VulkanCore.hpp"
    "include/pch.h"
    "include/PipelineSettings.hpp"
    "include/RenderContext.hpp"
    "include/Renderer.hpp"
    "include/RenderProgramFactory.hpp"
    "include/RenderWindow.hpp"
    "include/Resources/Buffer.hpp"
    "include/Resources/DynamicBuffer.hpp"
    "include/Resources/DynamicTexture.hpp"
    "include/Resources/ResourceManager.hpp"
    "include/Resources/Texture.hpp"
    "include/ShaderInfoStructs.h"
    "include/ShaderSet.hpp"
    "include/structs.hpp"
    "include/Tools/Logger.hpp"
    "include/Tools/utils.hpp"
    "include/Window.hpp"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/Buffer.cpp"
    "src/CommandPool.cpp"
    "src/debugFunctions.cpp"
    "src/DescriptorSet.cpp"
    "src/DeviceMemoryManager.cpp"
    "src/DynamicBuffer.cpp"
    "src/DynamicTexture.cpp"
    "src/FramebufferSet.cpp"
    "src/Image.cpp"
    "src/imgui.cpp"
    "src/imgui_demo.cpp"
    "src/imgui_draw.cpp"
    "src/imgui_impl_glfw.cpp"
    "src/imgui_impl_vulkan.cpp"
    "src/imgui_stdlib.cpp"
    "src/imgui_tables.cpp"
    "src/imgui_widgets.cpp"
    "src/Logger.cpp"
    "src/Pipeline.cpp"
    "src/RenderContext.cpp"
    "src/Renderer.cpp"
    "src/RenderProgram.cpp"
    "src/RenderProgramFactory.cpp"
    "src/RenderWindow.cpp"
    "src/ResourceManager.cpp"
    "src/ShaderInfoStructs.cpp"
    "src/ShaderSet.cpp"
    "src/Swapchain.cpp"
    "src/Texture.cpp"
    "src/utils.cpp"
    "src/VulkanCore.cpp"
    "src/Window.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE VulkanRenderer)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG     "${PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
    TARGET_NAME_OPTIMIZED "${PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
    TARGET_NAME_RELEASE   "${PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG     "${CMAKE_CURRENT_SOURCE_DIR}/Lib/"
    OUTPUT_DIRECTORY_OPTIMIZED "${CMAKE_CURRENT_SOURCE_DIR}/Lib/"
    OUTPUT_DIRECTORY_RELEASE   "${CMAKE_CURRENT_SOURCE_DIR}/Lib/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_OPTIMIZED "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE   "TRUE"
)

################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Optimized>:
            MultiThreadedDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Optimized>,$<CONFIG:Release>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include;"
    "$ENV{VULKAN_SDK}/Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/GLFW/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        "SA_RENDER_VALIDATION_ENABLE;"
        "SA_DEBUG_LOG_ENABLE"
    ">"
    "$<$<CONFIG:Optimized>:"
        "NDEBUG;"
        "SA_RENDER_VALIDATION_ENABLE;"
        "SA_DEBUG_LOG_ENABLE"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_CONSOLE;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Optimized>:
            /Oi;
            /Gy
        >
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Optimized>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:CONSOLE
    )
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "spirv-cross-cored;"
        "spirv-cross-cppd;"
        "spirv-cross-glsld;"
        "shaderc_combinedd"
    ">"
    "$<$<CONFIG:Optimized>:"
        "spirv-cross-core;"
        "spirv-cross-cpp;"
        "spirv-cross-glsl;"
        "shaderc_combined"
    ">"
    "$<$<CONFIG:Release>:"
        "spirv-cross-core;"
        "spirv-cross-cpp;"
        "spirv-cross-glsl;"
        "shaderc_combined"
    ">"
    "vulkan-1;"
    "glfw3"
)

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PUBLIC
    "$ENV{VULKAN_SDK}/Lib;"
    "${CMAKE_CURRENT_SOURCE_DIR}//deps/GLFW/lib-vc2019"
)


