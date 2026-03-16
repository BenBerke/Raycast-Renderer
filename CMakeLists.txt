cmake_minimum_required(VERSION 4.1)
project(Raycast_Renderer LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(Raycast_Renderer
        main.c
        config.h
        src/Systems/Renderer.c
        Headers/Systems/Renderer.h
        src/Tools/Vector2.c
        Headers/Tools/Vector2.h
        Headers/Objects/Wall.h
        Headers/Tools/Vector3.h
        src/Tools/Vector3.c
        src/Systems/InputManager.c
        Headers/Systems/InputManager.h
        src/Objects/Player.c
        Headers/Objects/Player.h
        src/Systems/Physics.c
        Headers/Systems/Physics.h
        src/Systems/Raycast.c
        Headers/Systems/Raycast.h
        src/Systems/TextureManager.c
        Headers/Systems/TextureManager.h
        src/Objects/Object.c
        Headers/Objects/Object.h
        Headers/Tools/Vector4.h
        src/Tools/Vector4.c
        Headers/AppState.h
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

target_link_libraries(Raycast_Renderer PRIVATE
        SDL3::SDL3
        $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
        SDL3_ttf::SDL3_ttf
)

# --- Shader compilation ---
find_program(SHADERCROSS NAMES shadercross shadercross.exe
        HINTS "C:/tools/shadercross/bin"
)

if(NOT SHADERCROSS)
    message(FATAL_ERROR "shadercross.exe not found")
endif()

set(HLSL_SHADERS
        "${CMAKE_SOURCE_DIR}/shaders/Raycast.vert.hlsl"
        "${CMAKE_SOURCE_DIR}/shaders/SolidColor.frag.hlsl"
)

set(COMPILED_SHADERS "")

foreach(SHADER IN LISTS HLSL_SHADERS)
    get_filename_component(SHADER_FILE "${SHADER}" NAME)
    string(REGEX REPLACE "\\.hlsl$" "" SHADER_BASE "${SHADER_FILE}")

    set(OUT_DXIL "${CMAKE_BINARY_DIR}/shaders/${SHADER_BASE}.dxil")

    add_custom_command(
            OUTPUT "${OUT_DXIL}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
            COMMAND "${SHADERCROSS}" "${SHADER}" -o "${OUT_DXIL}"
            DEPENDS "${SHADER}"
            COMMENT "Compiling ${SHADER_FILE} -> ${OUT_DXIL}"
            VERBATIM
    )

    list(APPEND COMPILED_SHADERS "${OUT_DXIL}")
endforeach()

add_custom_target(Shaders ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(Raycast_Renderer Shaders)