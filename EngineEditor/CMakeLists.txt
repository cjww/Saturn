set(PROJECT_NAME EngineEditor)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "src/CustomImGui.h"
    "src/DirectoryView.h"
    "src/EditorModule.h"
    "src/EngineEditor.h"
    "src/EntityInspector.h"
    "src/Events.h"
    "src/FileTemplates.h"
    "src/GameView.h"
    "src/ImGuiRenderLayer.h"
    "src/RenderPipelinePreferences.h"
    "src/SceneHierarchy.h"
    "src/SceneView.h"
    "src/TestLayer.h"
    "src/Tools/FileDialogs.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "deps/ImGuizmo/ImGuizmo.cpp"
    "src/CustomImGui.cpp"
    "src/DirectoryView.cpp"
    "src/EditorModule.cpp"
    "src/EngineEditor.cpp"
    "src/EntityInspector.cpp"
    "src/FileTemplates.cpp"
    "src/GameView.cpp"
    "src/ImGuiRenderLayer.cpp"
    "src/RenderPipelinePreferences.cpp"
    "src/SaturnEditorApp.cpp"
    "src/SceneHierarchy.cpp"
    "src/SceneView.cpp"
    "src/TestLayer.cpp"
    "src/Tools/FileDialogs.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE EngineEditor)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../VulkanRenderer/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/ImGuizmo"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG;"
        "SA_DEBUG_LOG_ENABLE;"
        "SA_PROFILER_ENABLE"
    ">"
    "$<$<CONFIG:Optimized>:"
        "NDEBUG;"
        "SA_DEBUG_LOG_ENABLE;"
        "SA_PROFILER_ENABLE"
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
        $<$<CONFIG:Debug>:
            /INCREMENTAL;
            /DEBUG
        >
        $<$<CONFIG:Optimized>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /SUBSYSTEM:CONSOLE
    )
    
endif()

################################################################################
# Dependencies
################################################################################
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Engine
)

# Copying dlls
add_custom_command ( TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
    COMMENT "Copying dlls $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
    VERBATIM
)
# Copying resources
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:${PROJECT_NAME},SOURCE_DIR>/resources"
        "$<TARGET_PROPERTY:${PROJECT_NAME},BINARY_DIR>/resources"
        
)

# Copying shaders
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_PROPERTY:Engine,SOURCE_DIR>/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
    COMMENT "Copying Shaders $<TARGET_PROPERTY:Engine,BINARY_DIR>/shaders to $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)