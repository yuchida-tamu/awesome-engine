# Unit tests: link the UNIT_TEST build of the engine (engine_test). Engine
# headers resolve through engine_test's PUBLIC include dirs; only doctest and
# the local tests/ dir (TestHelpers.h) need to be added here.
add_executable(run-tests
    InputTests.cpp
    CameraTests.cpp
    EventBusTests.cpp
    TransformComponentTests.cpp
    CameraControllerTests.cpp
    GameObjectTests.cpp
    SceneTests.cpp
    RenderContextTests.cpp
    UIElementTests.cpp
    UIManagerTests.cpp
    TextElementTests.cpp
    ButtonElementTests.cpp
    ChunkTests.cpp
    ChunkMesherTests.cpp
    GreedyMesherTests.cpp
    TerrainGeneratorTests.cpp
    CoordsTests.cpp
    FpsCounterTests.cpp
    ReconcileTests.cpp
    DesiredSetTests.cpp
    TimestepTests.cpp
    LayerStackTests.cpp
)
target_include_directories(run-tests PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/doctest
    ${CMAKE_CURRENT_SOURCE_DIR}            # TestHelpers.h
)
target_link_libraries(run-tests PRIVATE engine_test)
add_test(NAME AllTests COMMAND run-tests)
