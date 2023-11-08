set(PROJECT_NAME Engine)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "include/Application.h"
    "include/AssetManager.h"
    "include/Assets/Asset.h"
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
    "include/Graphics/RenderPipeline.h"
    "include/Graphics/RenderTarget.h"
    "include/Graphics/RenderTechniques/ForwardPlus.h"
    "include/Graphics/SceneCollection.h"
    "include/Graphics/WindowRenderer.h"
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
    "src/utils.cpp"
    "src/UUID.cpp"
    "src/WindowRenderer.cpp"
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
    TARGET_NAME_DEBUG     "$ENV{SolutionName}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
    TARGET_NAME_OPTIMIZED "$ENV{SolutionName}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
    TARGET_NAME_RELEASE   "$ENV{SolutionName}_${CMAKE_VS_PLATFORM_NAME}$<CONFIG>"
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
add_dependencies(${PROJECT_NAME}
    VulkanRenderer
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "PhysXPvdSDK_static_64;"
        "assimp-vc143-mtd"
    ">"
    "$<$<CONFIG:Optimized>:"
        "assimp-vc143-mt"
    ">"
    "$<$<CONFIG:Release>:"
        "assimp-vc143-mt"
    ">"
    "VulkanRenderer_$(Platform)$(Configuration);"
    "lua51;"
    "PhysXExtensions_static_64;"
    "PhysX_64;"
    "PhysXCommon_64;"
    "PhysXFoundation_64;"
    "PhysXCooking_64"
)

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PUBLIC
    "$<$<CONFIG:Debug>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/lib/$<CONFIG>;"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/debug"
    ">"
    "$<$<CONFIG:Optimized>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/lib/Release;"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release"
    ">"
    "$<$<CONFIG:Release>:"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/assimp-5.2.4/lib/$<CONFIG>;"
        "${CMAKE_CURRENT_SOURCE_DIR}/deps/PhysX/bin/release"
    ">"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/lua/;"
    "${CMAKE_SOURCE_DIR}/VulkanRenderer/lib"
)


