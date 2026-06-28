# Game executable: a thin consumer of the `engine` library. main() comes from
# the engine's core/EntryPoint.h, instantiated exactly once via GameApp.cpp.
add_executable(awesome-engine
    GameApp.cpp
    GameLayer.cpp
)
target_include_directories(awesome-engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # "GameLayer.h"
target_link_libraries(awesome-engine PRIVATE engine)
# Engine declares its assimp-using code but not the link; the exe owns assimp
# (and the macOS RPATH/copy/sign below).
if(assimp_FOUND)
    target_link_libraries(awesome-engine PRIVATE assimp::assimp)
endif()

# --- Copy assets next to the binary (post-build) ---
set(SHADER_OUTPUT_DIR  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
set(TEXTURE_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/textures)
set(FONT_OUTPUT_DIR    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts)
set(MODEL_OUTPUT_DIR   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/models)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR} ${TEXTURE_OUTPUT_DIR} ${FONT_OUTPUT_DIR} ${MODEL_OUTPUT_DIR})

file(GLOB SHADER_FILES  "${CMAKE_SOURCE_DIR}/assets/shaders/*")
file(GLOB TEXTURE_FILES "${CMAKE_SOURCE_DIR}/assets/textures/*.png" "${CMAKE_SOURCE_DIR}/assets/textures/*.jpg")
file(GLOB FONT_FILES    "${CMAKE_SOURCE_DIR}/assets/fonts/*.ttf" "${CMAKE_SOURCE_DIR}/assets/fonts/*.otf")

foreach(SHADER_FILE ${SHADER_FILES})
    add_custom_command(TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_FILE} ${SHADER_OUTPUT_DIR})
endforeach()
foreach(TEXTURE_FILE ${TEXTURE_FILES})
    add_custom_command(TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${TEXTURE_FILE} ${TEXTURE_OUTPUT_DIR})
endforeach()
foreach(FONT_FILE ${FONT_FILES})
    add_custom_command(TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${FONT_FILE} ${FONT_OUTPUT_DIR})
endforeach()
add_custom_command(TARGET awesome-engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/models ${MODEL_OUTPUT_DIR}
    COMMENT "Copying model files to bin directory")

# macOS: copy the Assimp dylib next to the binary, re-sign it, and point the
# binary at @executable_path so it loads without DYLD path hunting.
if(APPLE AND assimp_FOUND)
    execute_process(COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
        COMMAND python3 -c "import os; print(os.path.realpath('${HOMEBREW_PREFIX}/lib/libassimp.5.dylib'))"
        OUTPUT_VARIABLE ASSIMP_LIB_PATH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(NOT ASSIMP_LIB_PATH)
        set(ASSIMP_LIB_PATH ${HOMEBREW_PREFIX}/lib/libassimp.5.dylib)
    endif()
    add_custom_command(TARGET awesome-engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSIMP_LIB_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libassimp.5.dylib
        COMMENT "Copying Assimp library to bin directory")
    add_custom_command(TARGET awesome-engine POST_BUILD
        COMMAND codesign --force --sign - ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libassimp.5.dylib
        COMMENT "Re-signing Assimp library")
    set_target_properties(awesome-engine PROPERTIES
        INSTALL_RPATH "@executable_path" BUILD_WITH_INSTALL_RPATH TRUE)
    add_custom_command(TARGET awesome-engine POST_BUILD
        COMMAND install_name_tool -change "@rpath/libassimp.5.dylib" "@executable_path/libassimp.5.dylib" $<TARGET_FILE:awesome-engine>
        COMMENT "Updating library path in binary")
endif()
