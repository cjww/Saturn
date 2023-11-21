set(PROJECT_NAME EngineTest)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "TestLayer.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "main.cpp"
    "TestLayer.cpp"
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
set(ROOT_NAMESPACE EngineTest)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_OPTIMIZED "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_RELEASE   "TRUE"
)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../VulkanRenderer/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Optimized>:"
        "NDEBUG;"
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
        /std:c++17;
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
add_dependencies(${PROJECT_NAME}
    Engine
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$(SolutionName)_$(Platform)$(Configuration)"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}//Engine/lib"
)
