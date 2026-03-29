#include <math.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>
#include <string.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Raycast.h"

static Uint32 fpsTimer = 0;
static int frames = 0;
static float currentFps = 0.0f;

static void movement(const InputManager* inputManager, Player* player, float deltaTime) {
    const Vector2 forward = { cosf(player->angle), sinf(player->angle) };
    const Vector2 right = { -sinf(player->angle), cosf(player->angle) };

    float moveAmount = 100.0f * deltaTime;

    if (input_manager_get_key(inputManager, SDL_SCANCODE_W)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(forward, moveAmount));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_S)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(forward, -moveAmount));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_D)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(right, moveAmount));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_A)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(right, -moveAmount));
    }

    if (input_manager_get_key(inputManager, SDL_SCANCODE_RIGHT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_E)) {
        player->angle += PLAYER_ROT_SPEED * deltaTime;
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_LEFT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_Q)) {
        player->angle -= PLAYER_ROT_SPEED * deltaTime;
    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    SDL_Window* window = SDL_CreateWindow("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return -1;
    }

    Renderer renderer = {0};
    if (!renderer_init(&renderer, window)) {
        return -1;
    }

    Player player = { .position = {0, 0}, .scale = 15.0f, .velocity = {0, 0}, .speed = .1f, .friction = 3.0f, .angle = 0 };
    InputManager inputManager = {0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);
    Wall walls[] = {
        {
            .position = {100.0f, 0.0f},
            .scale    = {10.0f, 120.0f},
            .color    = {255, 255, 255, 255},
            .textures = {0, 0, 0, 0},
            .height   = 150.0f,
            .faceBrightness = {0, 0, 0, 0}
        },
    };
    for (int i = 0; i < sizeof(walls) / sizeof(walls[0]); i++) {
        physics_push_walls_list(&wallsList, &walls[i]);
    }

    bool running = true;
    Uint64 lastTime = SDL_GetTicksNS();
    while (running) {
        Uint64 currentTime = SDL_GetTicksNS();
        float deltaTime = (float)(currentTime - lastTime) / 1000000000.0f;
        lastTime = currentTime;

        input_manager_begin_frame(&inputManager);

        if (input_manager_get_key_down(&inputManager, SDL_SCANCODE_ESCAPE)) running = false;
        movement(&inputManager, &player, deltaTime);
        player_update(&player);

        RaySlice cpuSlices[RAY_COUNT];
        raycast_to_gpu_buffer(&player, &wallsList, cpuSlices);

        renderer_render_frame(&renderer, window, cpuSlices);
    }

    renderer_destroy(&renderer);

    SDL_DestroyWindow(window);
    physics_free_walls_list(&wallsList);
    SDL_Quit();
    return 0;
}