cmake_minimum_required(VERSION 3.24)

project(Saturn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Nice folder grouping in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Warnings
if(MSVC)
    add_compile_options(/W3 /bigobj)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencies
include(FetchContent)

# Assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.4
)

# Disable all Assimp tools/tests/install
set(ASSIMP_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL          OFF CACHE BOOL "" FORCE)

# Optional: build static only (faster and simpler linking)
set(BUILD_SHARED_LIBS       OFF CACHE BOOL "" FORCE)

# Optional: disable exporters you don't need
set(ASSIMP_BUILD_GLTF_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_COLLADA_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_EXPORTER OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(assimp)

# Lua
FetchContent_Declare(
        lua
        GIT_REPOSITORY https://github.com/walterschell/Lua.git
        GIT_TAG v5.4.7
)
FetchContent_MakeAvailable(lua)

message(STATUS "VULKAN_SDK = $ENV{VULKAN_SDK}")
# Vulkan
find_package(Vulkan REQUIRED)

# SPIRV-Headers
FetchContent_Declare(
        spirv_headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG main
)
FetchContent_MakeAvailable(spirv_headers)

# SPIRV-Tools
FetchContent_Declare(
        spirv_tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG main
)
FetchContent_MakeAvailable(spirv_tools)

# glslang
FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG main
)
FetchContent_MakeAvailable(glslang)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2024.3
        GIT_SHALLOW TRUE
        GIT_SUBMODULES_RECURSE TRUE
)
FetchContent_MakeAvailable(shaderc)

FetchContent_Declare(
        spirv_cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG main
)
FetchContent_MakeAvailable(spirv_cross)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

# Physx

#set(PHYSX_ROOT "Engine/deps/PhysX")
#add_library(PhysX STATIC IMPORTED)
#
#set_target_properties(PhysX PROPERTIES
#    IMPORTED_LOCATION_DEBUG   "${PHYSX_ROOT}/bin/debug/PhysX_64.lib"
#    IMPORTED_LOCATION_RELEASE "${PHYSX_ROOT}/bin/release/PhysX_64.lib"
#)
#
#add_library(PhysXCommon STATIC IMPORTED)
#
#set_target_properties(PhysXCommon PROPERTIES
#        IMPORTED_LOCATION_DEBUG   "${PHYSX_ROOT}/bin/debug/PhysXCommon_64.lib"
#        IMPORTED_LOCATION_RELEASE "${PHYSX_ROOT}/bin/release/PhysXCommon_64.lib"
#)
#
#add_library(PhysXFoundation STATIC IMPORTED)
#
#set_target_properties(PhysXFoundation PROPERTIES
#        IMPORTED_LOCATION_DEBUG   "${PHYSX_ROOT}/bin/debug/PhysXFoundation_64.lib"
#        IMPORTED_LOCATION_RELEASE "${PHYSX_ROOT}/bin/release/PhysXFoundation_64.lib"
#)
# Engine modules
add_subdirectory(VulkanRenderer)
add_subdirectory(VulkanRendererTest)
add_subdirectory(Engine)
add_subdirectory(EngineTest)
add_subdirectory(EngineEditor)