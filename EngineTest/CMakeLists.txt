add_executable(EngineTest
        main.cpp
        TestLayer.cpp
)

target_link_libraries(EngineTest
        PRIVATE
        Engine
)

add_custom_command(TARGET EngineTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:EngineTest>
        $<TARGET_FILE_DIR:EngineTest>
        COMMAND_EXPAND_LISTS
)