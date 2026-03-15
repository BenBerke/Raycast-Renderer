#include <math.h>
#include <stdio.h>
#include <SDL3/SDL.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"
#include "Headers/Systems/TextureManager.h"
#include "Headers/Systems/Debugger.h"

#include "Headers/Objects/Object.h"
#include "Headers/Objects/Light.h"


static Uint32 fpsTimer = 0;
static int frames = 0;
static float currentFps = 0.0f;

static void movement(const InputManager* inputManager, Player* player, float deltaTime) {
    const Vector2 forward = { cosf(player->angle), sinf(player->angle) };
    const Vector2 right = { -sinf(player->angle), cosf(player->angle) };

    if (input_manager_get_key(inputManager, SDL_SCANCODE_W)) {
        player_add_velocity(player, vector2_multiply_with_float(forward, player->speed * deltaTime));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_S)) {
        player_add_velocity(player, vector2_multiply_with_float(forward, -player->speed * deltaTime ));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_D)) {
        player_add_velocity(player, vector2_multiply_with_float(right, -player->speed * deltaTime));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_A)) {
        player_add_velocity(player, vector2_multiply_with_float(right, player->speed * deltaTime));
    }

    if (input_manager_get_key(inputManager, SDL_SCANCODE_RIGHT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_E)) {
        player->angle -= PLAYER_ROT_SPEED * deltaTime;
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_LEFT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_Q)) {
        player->angle += PLAYER_ROT_SPEED * deltaTime;
    }
}

static int initialize(SDL_Window** window, SDL_Renderer** renderer) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    *window = SDL_CreateWindow("SDL3 Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!*window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *renderer = SDL_CreateRenderer(*window, NULL);
    if (!*renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 1;
    }

    return 0;
}

int main(void) {
    bool debug = false;

    SDL_Window* window = NULL;
    SDL_Renderer* sdlRenderer = NULL;
    if (initialize(&window, &sdlRenderer) != 0) {
        return 1;
    }

    TexturesList textures;
    textureManager_create_textures_list(&textures,  8);

    InputManager inputManager = {0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);

    ObjectsList objectsList;
    objects_create_objects_list(&objectsList, 8);

    Wall walls[] = {
            {{-260, 220}, {220, 40}, {255, 255, 255, 255}, {wallTexture, wallTexture, wallTexture, wallTexture}, 2.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
            {{-100, 320}, {220, 40}, {255, 255, 255, 255}, {whiteTexture, whiteTexture, whiteTexture, whiteTexture}, 2.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
    };

    Object objects[] = {
        {{0, 0}, {1, 1},{255, 255, 255, 255}, humanTexture},
    };

    const int wallCount = sizeof(walls) / sizeof(walls[0]);
    for (int i = 0; i < wallCount; i++) {
        physics_push_walls_list(&wallsList, &walls[i]);
    }
    const int objectCount = sizeof(objects) / sizeof(objects[0]);
    for (int i = 0; i < objectCount; i++) {
        objects_push_objects_list(&objectsList, &objects[i]);
    }
    Player player = {{0, 0}, PLAYER_SCALE, {0, 0}, PLAYER_SPEED, PLAYER_FRICTION, 0};

    bool running = true;
    Uint64 lastTime = SDL_GetTicksNS();
    while (running) {
        Uint64 currentTime = SDL_GetTicksNS();
        Uint32 startTime = SDL_GetTicks();
        float deltaTime = (float)(currentTime - lastTime) / 1000000000.0f;
        lastTime = currentTime;

        input_manager_begin_frame(&inputManager);
        if (input_manager_get_key_down(&inputManager, SDL_SCANCODE_ESCAPE)) {
            running = false;
        }

        movement(&inputManager, &player, deltaTime);
        debug = input_manager_get_key(&inputManager, SDL_SCANCODE_TAB);

        player_update(&player);
        physics_check_collisions(&player, &wallsList);


        Uint32 frameTime = SDL_GetTicks() - startTime;
        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }

        frames++;
        Uint32 now = SDL_GetTicks();
        if (now - fpsTimer >= 1000) {
            currentFps = frames * 1000.0f / (float)(now - fpsTimer);
            fpsTimer = now;
            frames = 0;
        }
    }

    physics_free_walls_list(&wallsList);
    textureManager_free_textures_list(&textures);
    objects_free_objects_list(&objectsList);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}