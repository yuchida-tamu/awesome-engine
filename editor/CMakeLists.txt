# Editor executable: a thin consumer of `engine` (which links ImGui PUBLIC, so
# the editor gets ImGui transitively). For now it just shows the ImGui demo;
# the render-to-texture viewport and panels come in E3/E4. No asset/RPATH copy
# yet — it's ImGui-only and doesn't render the world.
add_executable(awesome-engine-editor
    EditorApp.cpp
    EditorLayer.cpp
)
target_include_directories(awesome-engine-editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(awesome-engine-editor PRIVATE engine)
