set(PROJECT_NAME Engine)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "include/Application.h"
    "include/AssetManager.h"
    "include/Assets/Asset.h"
    "include/Assets/AssetHolder.h"
    "include/Assets/MaterialShader.h"
    "include/Assets/ModelAsset.h"
    "include/Assets/TextureAsset.h"
    "include/Core.h"
    "include/ECS/ComponentBase.h"
    "include/ECS/Components.h"
    "include/ECS/Components/BoxCollider.h"
    "include/ECS/Components/Camera.h"
    "include/ECS/Components/Light.h"
    "include/ECS/Components/Model.h"
    "include/ECS/Components/Name.h"
    "include/ECS/Components/RigidBody.h"
    "include/ECS/Components/SphereCollider.h"
    "include/ECS/Components/Transform.h"
    "include/ECS/Components/ShadowEmitter.h"
    "include/ECS/ComponentType.h"
    "include/ECS/Entity.h"
    "include/ECS/Events.h"
    "include/ECS/MetaComponent.h"
    "include/Engine.h"
    "include/EntityHierarchy.h"
    "include/EntryPoint.h"
    "include/Graphics/IRenderLayer.h"
    "include/Graphics/IRenderTechnique.h"
    "include/Graphics/Material.h"
    "include/Graphics/RenderLayers/BloomRenderLayer.h"
    "include/Graphics/RenderLayers/ShadowRenderLayer.h"
    "include/Graphics/RenderPipeline.h"
    "include/Graphics/RenderTarget.h"
    "include/Graphics/RenderTechniques/ForwardPlus.h"
    "include/Graphics/SceneCollection.h"
    "include/Graphics/WindowRenderer.h"
    "include/Graphics/DebugRenderer.h"
    "include/Lua/EntityScript.h"
    "include/Lua/LuaAccessable.h"
    "include/Lua/Ref.h"
    "include/Lua/ScriptManager.h"
    "include/pch.h"
    "include/PhysicsSystem.h"
    "include/ProgressView.h"
    "include/Scene.h"
    "include/SceneCamera.h"
    "include/Serializable.h"
    "include/simdjson.h"
    "include/structs.h"
    "include/Tools/Clock.h"
    "include/Tools/Math.h"
    "include/Tools/Matrix.h"
    "include/Tools/MemoryChecker.h"
    "include/Tools/Profiler.h"
    "include/Tools/ScopeTimer.h"
    "include/Tools/StopWatch.h"
    "include/Tools/utils.h"
    "include/Tools/Vector.h"
    "include/UUID.h"
    "include/Vertex.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/Application.cpp"
    "src/Asset.cpp"
    "src/AssetManager.cpp"
    "src/BloomRenderLayer.cpp"
    "src/ShadowRenderLayer.cpp"
    "src/BoxCollider.cpp"
    "src/Camera.cpp"
    "src/Clock.cpp"
    "src/Components.cpp"
    "src/ComponentType.cpp"
    "src/Engine.cpp"
    "src/Entity.cpp"
    "src/EntityHierarchy.cpp"
    "src/EntityScript.cpp"
    "src/ForwardPlus.cpp"
    "src/IRenderLayer.cpp"
    "src/IRenderTechnique.cpp"
    "src/Light.cpp"
    "src/LuaAccessable.cpp"
    "src/Material.cpp"
    "src/MaterialShader.cpp"
    "src/Math.cpp"
    "src/MemoryChecker.cpp"
    "src/MetaComponent.cpp"
    "src/Model.cpp"
    "src/ModelAsset.cpp"
    "src/Name.cpp"
    "src/pch.cpp"
    "src/PhysicsSystem.cpp"
    "src/Profiler.cpp"
    "src/Ref.cpp"
    "src/RenderPipeline.cpp"
    "src/RenderTarget.cpp"
    "src/RigidBody.cpp"
    "src/Scene.cpp"
    "src/SceneCamera.cpp"
    "src/SceneCollection.cpp"
    "src/ScopeTimer.cpp"
    "src/ScriptManager.cpp"
    "src/Serializable.cpp"
    "src/simdjson.cpp"
    "src/SphereCollider.cpp"
    "src/StopWatch.cpp"
    "src/TextureAsset.cpp"
    "src/Transform.cpp"
    "src/ShadowEmitter.cpp"
    "src/utils.cpp"
    "src/UUID.cpp"
    "src/WindowRenderer.cpp"
    "src/DebugRenderer.cpp"
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
set(ROOT_NAMESPACE Engine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG     "${CMAKE_PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
    TARGET_NAME_OPTIMIZED "${CMAKE_PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
    TARGET_NAME_RELEASE   "${CMAKE_PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG     "${CMAKE_CURRENT_SOURCE_DIR}/lib/"
    OUTPUT_DIRECTORY_OPTIMIZED "${CMAKE_CURRENT_SOURCE_DIR}/lib/"
    OUTPUT_DIRECTORY_RELEASE   "${CMAKE_CURRENT_SOURCE_DIR}/lib/"
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
        $<$<NOT:$<OR:$<CONFIG:Debug>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../VulkanRenderer/include"
)


################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "SA_DEBUG_LOG_ENABLE;"
        "SA_PROFILER_ENABLE;"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Optimized>:"
        "SA_DEBUG_LOG_ENABLE;"
        "SA_PROFILER_ENABLE;"
        "NDEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "_CONSOLE;"
    "_SILENCE_CXX20_CISO646_REMOVED_WARNING;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od;
            /Gy
        >
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
        /bigobj;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
        
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:    
            /DEBUG
        >
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

target_link_libraries(${PROJECT_NAME} PUBLIC
    VulkanRenderer
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "PhysXPvdSDK_static_64;"
    ">"
    "PhysXExtensions_static_64;"
)


target_link_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug"
    ">"
    "$<$<CONFIG:Optimized>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release"
    ">"
)

# Lua
add_library(lua SHARED IMPORTED)
set_target_properties(lua PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua/lua51.dll)
set_target_properties(lua PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua/lua51.lib)

# Assimp
add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES 
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/bin/Debug/assimp-vc143-mtd.dll
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/lib/Debug/assimp-vc143-mtd.lib
   
    IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/bin/Release/assimp-vc143-mt.dll
    IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/lib/Release/assimp-vc143-mt.lib
   
    IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/bin/Release/assimp-vc143-mt.dll
    IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/lib/Release/assimp-vc143-mt.lib
)

# PhysX
add_library(PhysX::Common SHARED IMPORTED)
set_target_properties(PhysX::Common PROPERTIES 
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCommon_64.dll
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCommon_64.lib
    
    IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCommon_64.dll
    IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCommon_64.lib
    
    IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCommon_64.dll
    IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCommon_64.lib
)
add_library(PhysX::Foundation SHARED IMPORTED)
set_target_properties(PhysX::Foundation PROPERTIES 
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXFoundation_64.dll
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXFoundation_64.lib
    
    IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXFoundation_64.dll
    IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXFoundation_64.lib
    
    IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXFoundation_64.dll
    IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXFoundation_64.lib
)

add_library(PhysX::Cooking SHARED IMPORTED)
set_target_properties(PhysX::Cooking PROPERTIES 
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCooking_64.dll
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysXCooking_64.lib
    
    IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCooking_64.dll
    IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCooking_64.lib
    
    IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCooking_64.dll
    IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysXCooking_64.lib
)
add_library(PhysX SHARED IMPORTED)
set_target_properties(PhysX PROPERTIES 
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysX_64.dll
    IMPORTED_IMPLIB_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug/PhysX_64.lib
    
    IMPORTED_LOCATION_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysX_64.dll
    IMPORTED_IMPLIB_OPTIMIZED ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysX_64.lib
    
    IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysX_64.dll
    IMPORTED_IMPLIB_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release/PhysX_64.lib
)
set_property(TARGET PhysX PROPERTY INTERFACE_LINK_LIBRARIES 
    PhysX::Common
    PhysX::Foundation
    PhysX::Cooking
)


set(target_shared_libs
    lua
    assimp
    PhysX
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${target_shared_libs} "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

# unset(glslc_executable CACHE)
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

#==============================================================================
# COMPILE SHADERS
#
# Include Shaders
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

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
      --target-env=vulkan1.3
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling shader ${FILENAME} with ${glslc_executable} into ${SHADER_BINARY_DIR}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

add_dependencies(${PROJECT_NAME}
    shaders
)
