add_library(Engine STATIC)

file(GLOB_RECURSE ENGINE_SRC
        src/*.cpp
)
list(FILTER ENGINE_SRC EXCLUDE REGEX "src/ForwardRenderer.cpp")

file(GLOB_RECURSE ENGINE_HEADERS
        include/*.h
        include/*.hpp
)
list(FILTER ENGINE_HEADERS EXCLUDE REGEX "include/Graphics/RenderTechniques/ForwardRenderer.h")

target_sources(Engine
        PRIVATE
        ${ENGINE_SRC}
        ${ENGINE_HEADERS}
)

target_include_directories(Engine
        PUBLIC
        include
)

add_library(PhysX::Common SHARED IMPORTED)
set_target_properties(PhysX::Common PROPERTIES
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCommon_64.dll
        IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCommon_64.lib

        IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXCommon_64.dll
        IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXCommon_64.lib

        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCommon_64.dll
        IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCommon_64.lib
)
add_library(PhysX::Foundation SHARED IMPORTED)
set_target_properties(PhysX::Foundation PROPERTIES
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXFoundation_64.dll
        IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXFoundation_64.lib

        IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXFoundation_64.dll
        IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXFoundation_64.lib

        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXFoundation_64.dll
        IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXFoundation_64.lib
)

add_library(PhysX::Cooking SHARED IMPORTED)
set_target_properties(PhysX::Cooking PROPERTIES
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCooking_64.dll
        IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCooking_64.lib

        IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXCooking_64.dll
        IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXCooking_64.lib

        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCooking_64.dll
        IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCooking_64.lib
)

add_library(PhysX::Extensions STATIC IMPORTED)
set_target_properties(PhysX::Extensions PROPERTIES
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXExtensions_static_64.lib
        IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXExtensions_static_64.lib
        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXExtensions_static_64.lib
)

add_library(PhysX::PvdSDK STATIC IMPORTED)
set_target_properties(PhysX::PvdSDK PROPERTIES
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXPvdSDK_static_64.lib
        IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysXPvdSDK_static_64.lib
        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXPvdSDK_static_64.lib
)

add_library(PhysX SHARED IMPORTED)
set_target_properties(PhysX PROPERTIES
        IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysX_64.dll
        IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysX_64.lib

        IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysX_64.dll
        IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/checked/PhysX_64.lib

        IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysX_64.dll
        IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysX_64.lib

        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/include"

)
set_property(TARGET PhysX PROPERTY INTERFACE_LINK_LIBRARIES
        PhysX::Common
        PhysX::Foundation
        PhysX::Cooking
        PhysX::Extensions
        PhysX::PvdSDK
)

target_link_libraries(Engine
        PUBLIC
        VulkanRenderer
        assimp
        Lua::Library
        PhysX
)

target_precompile_headers(Engine
        PRIVATE
        include/pch.h
)

set_target_properties(Engine PROPERTIES UNITY_BUILD ON)

target_compile_definitions(Engine PRIVATE
        $<$<CONFIG:Debug>:SA_DEBUG_LOG_ENABLE>
        $<$<CONFIG:Debug>:SA_PROFILER_ENABLE>
)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

#==============================================================================
# COMPILE SHADERS
#
# Include Shaders
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
target_compile_definitions(Engine PRIVATE SA_DEFAULT_SHADER_DIR="${SHADER_BINARY_DIR}")

file(GLOB SHADERS
        ${SHADER_SOURCE_DIR}/*.vert
        ${SHADER_SOURCE_DIR}/*.frag
        ${SHADER_SOURCE_DIR}/*.comp
        ${SHADER_SOURCE_DIR}/*.geom
        ${SHADER_SOURCE_DIR}/*.tesc
        ${SHADER_SOURCE_DIR}/*.tese
        ${SHADER_SOURCE_DIR}/*.mesh
        ${SHADER_SOURCE_DIR}/*.task
        ${SHADER_SOURCE_DIR}/*.rgen
        ${SHADER_SOURCE_DIR}/*.rchit
        ${SHADER_SOURCE_DIR}/*.rmiss)

source_group("Shaders" FILES ${SHADERS})

add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
        OUTPUT ${SHADER_BINARY_DIR}
        COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
            COMMAND
            ${glslc_executable}
            #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
            --target-env=vulkan1.4
            -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
            ${source}
            OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
            DEPENDS ${source} ${SHADER_BINARY_DIR}
            COMMENT "Compiling shader ${FILENAME} with ${glslc_executable} into ${SHADER_BINARY_DIR}"
    )
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders_compile_target ALL DEPENDS ${SPV_SHADERS})

add_dependencies(Engine
        shaders_compile_target
)