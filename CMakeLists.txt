cmake_minimum_required(VERSION 4.1)
project(Raycast_Renderer LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(Raycast_Renderer
        main.c
        config.h
        src/Systems/Renderer.c
        Headers/Systems/Renderer.h
        src/Math/Vector2.c
        Headers/Tools/Vector2.h
        Headers/Objects/Wall.h
        Headers/Tools/Vector3.h
        src/Math/Vector3.c
        src/Systems/InputManager.c
        Headers/Systems/InputManager.h
        src/Objects/Player.c
        Headers/Objects/Player.h
        src/Systems/Physics.c
        Headers/Systems/Physics.h
        src/Systems/Raycast.c
        Headers/Systems/Raycast.h
        src/Objects/DebugSquare.c
        Headers/Objects/DebugSquare.h
        Headers/Objects/Ray.h
        src/Systems/TextureManager.c
        Headers/Systems/TextureManager.h
)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

target_link_libraries(Raycast_Renderer PRIVATE
        SDL3::SDL3
        $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
        SDL3_ttf::SDL3_ttf
)