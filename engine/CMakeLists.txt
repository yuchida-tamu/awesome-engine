# Engine: a reusable static library (no main()). Built twice — `engine`
# (production) and `engine_test` (compiled with UNIT_TEST) for the headless
# test runner. Sources/includes are shared between the two via these variables.
set(ENGINE_SOURCES
    ${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c
    ${CMAKE_SOURCE_DIR}/vendor/loader/src/stb_image.cpp
    cameras/Camera.cpp
    core/Input.cpp
    core/Window.cpp
    core/TextureLoader.cpp
    core/LayerStack.cpp
    core/Application.cpp
    core/ImGuiLayer.cpp
    core/FrameBuffer.cpp
    meshes/Model.cpp
    meshes/Mesh.cpp
    meshes/Cube.cpp
    rendering/PostProcessing.cpp
    rendering/PostProcessBlurEffectStrategy.cpp
    rendering/PostProcessInvertEffectStrategy.cpp
    rendering/PostProcessEdgeEffectStrategy.cpp
    rendering/Skybox.cpp
    rendering/Shader.cpp
    scene/WorldSpaceGizmo.cpp
    scene/GridGizmo.cpp
    scene/Scene.cpp
    scene/GameObject.cpp
    scene/RenderComponent.cpp
    scene/CameraController.cpp
    ui/UIManager.cpp
    ui/TextElement.cpp
    ui/ButtonElement.cpp
    ui/DebugPanel.cpp
    voxel/VoxelChunk.cpp
    voxel/ChunkMesher.cpp
    voxel/GreedyMesher.cpp
    voxel/TerrainGenerator.cpp
    world/World.cpp
)

set(ENGINE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}                  # engine/ is the include root: "core/...", "world/..."
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/loader/include
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/FastNoiseLite
)

add_library(engine STATIC ${ENGINE_SOURCES})
target_include_directories(engine PUBLIC ${ENGINE_INCLUDES})
target_link_libraries(engine PUBLIC glfw OpenGL::GL Freetype::Freetype imgui)
if(assimp_FOUND)
    target_include_directories(engine PUBLIC ${assimp_INCLUDE_DIRS})
endif()
# assimp is linked by the game exe, not here, so run-tests stays assimp-free.

# Test build of the engine: same sources, UNIT_TEST defined so the in-source
# test guards are active (UI ctors skip GL/shader creation, privates exposed,
# asserts throw). The production `engine` above stays clean.
if(BUILD_TESTING)
    add_library(engine_test STATIC ${ENGINE_SOURCES})
    target_include_directories(engine_test PUBLIC ${ENGINE_INCLUDES})
    target_link_libraries(engine_test PUBLIC glfw OpenGL::GL Freetype::Freetype imgui)
    target_compile_definitions(engine_test PUBLIC UNIT_TEST)
    if(assimp_FOUND)
        target_include_directories(engine_test PUBLIC ${assimp_INCLUDE_DIRS})
    endif()
endif()
