add_executable(EngineEditor)

file(GLOB_RECURSE ENGINE_EDITOR_SRC
        src/*.cpp
        deps/ImGuizmo/*.cpp
)

file(GLOB_RECURSE ENGINE_EDITOR_HEADERS
        src/*.h
        deps/ImGuizmo/*.h
)

target_sources(EngineEditor
        PRIVATE
        ${ENGINE_EDITOR_SRC}
        ${ENGINE_EDITOR_HEADERS}
)

target_include_directories(EngineEditor
        PUBLIC
        src
        deps/ImGuizmo
)

target_link_libraries(EngineEditor
        PUBLIC
        Engine
)

add_custom_command(TARGET EngineEditor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:EngineEditor>
        $<TARGET_FILE_DIR:EngineEditor>
        COMMAND_EXPAND_LISTS
)