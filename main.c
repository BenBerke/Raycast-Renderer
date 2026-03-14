#include <math.h>
#include <SDL3/SDL.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"
#include "Headers/Systems/TextureManager.h"
#include "Headers/Objects/Object.h"


static Uint32 fpsTimer = 0;
static int frames = 0;
static float currentFps = 0.0f;

static void movement(const InputManager* inputManager, Player* player) {
    const Vector2 forward = { cosf(player->angle), sinf(player->angle) };
    const Vector2 right = { -sinf(player->angle), cosf(player->angle) };

    if (input_manager_get_key(inputManager, SDL_SCANCODE_W)) {
        player_add_velocity(player, vector2_multiply_with_float(forward, player->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_S)) {
        player_add_velocity(player, vector2_multiply_with_float(forward, -player->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_D)) {
        player_add_velocity(player, vector2_multiply_with_float(right, -player->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_A)) {
        player_add_velocity(player, vector2_multiply_with_float(right, player->speed));
    }

    if (input_manager_get_key(inputManager, SDL_SCANCODE_RIGHT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_E)) {
        player->angle -= PLAYER_ROT_SPEED;
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_LEFT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_Q)) {
        player->angle += PLAYER_ROT_SPEED;
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

    Renderer renderer = create_renderer(window, sdlRenderer);

    TexturesList textures;
    textureManager_create_textures_list(&textures,  8);

    int skyBoxTexture = create_texture("skybox", &textures, renderer.renderer);
    int wallTexture = create_texture("wall", &textures, renderer.renderer);
    int woodTexture = create_texture("wood", &textures, renderer.renderer);
    int humanTexture = create_texture("human", &textures, renderer.renderer);

    const float skyDstHeight = SCREEN_HEIGHT;
    const float skyDstWidth = textures.items[skyBoxTexture].width * SCREEN_HEIGHT / textures.items[skyBoxTexture].height;
    const SDL_FRect skyBoxRect = {0.0f, 0.0f, skyDstWidth, skyDstHeight};

    InputManager inputManager = {0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);

    DebugSquaresList debugSquaresList;
    render_create_debugSquares_list(&debugSquaresList, 8);

    ObjectsList objectsList;
    objects_create_objects_list(&objectsList, 8);

    Wall walls[] = {
           {{-260, 220}, {220, 40}, {255, 80, 80}, wallTexture, 2.0f},
         {{40, 220},   {180, 40}, {255, 170, 60}, woodTexture, 1.5f},
           {{280, 220},  {140, 40}, {255, 230, 90}, wallTexture, 1.0f},

        {{-320, 120}, {40, 160}, {80, 220, 120}, woodTexture, .8f},
        {{-220, 100}, {140, 40}, {70, 180, 255}, wallTexture, 1.6f},
        {{-120, 20},  {40, 120}, {90, 120, 255}, woodTexture, 8.0f},
       {{-260, -40}, {180, 40}, {140, 100, 255}, wallTexture, 1.2f},
    };

    Object objects[] = {
        {{0, 0}, {10, 10},{255, 255, 255}, humanTexture},
{{0, 190}, {10, 10},{255, 255, 255}, humanTexture}
    };

    const int wallCount = sizeof(walls) / sizeof(walls[0]);
    for (int i = 0; i < wallCount; i++) {
        physics_push_walls_list(&wallsList, &walls[i]);
    }
    const int objectCount = sizeof(objects) / sizeof(objects[0]);
    for (int i = 0; i < objectCount; i++) {
        objects_push_objects_list(&objectsList, &objects[i]);
    }

    for (int i = 0; i < RAY_COUNT; i++) {
        DebugSquare square = {{0, 0}, {20, 20}, {0, 0, 255}};
        render_push_debugSquares_list(&debugSquaresList, &square);
    }

    Player player = {{0, 0}, PLAYER_SCALE, {0, 0}, PLAYER_SPEED, PLAYER_FRICTION, 0};

    bool running = true;
    while (running) {
        Uint32 startTime = SDL_GetTicks();

        input_manager_begin_frame(&inputManager);

        if (input_manager_get_key_down(&inputManager, SDL_SCANCODE_ESCAPE)) {
            running = false;
        }

        movement(&inputManager, &player);
        debug = input_manager_get_key(&inputManager, SDL_SCANCODE_TAB);

        player_update(&player);
        physics_check_collisions(&player, &wallsList);

        begin_frame(&renderer, textures.items[skyBoxTexture].texture, &skyBoxRect);

        renderer_draw(
            &textures,
            &player,
            &wallsList,
            &debugSquaresList,
            &renderer,
            &objectsList
        );

        if (debug) {
            render_debug_walls(&renderer, &wallsList);
            render_debug_player(&renderer, &player);
            render_debug_squares(&renderer, &debugSquaresList);
        }

        SDL_SetRenderDrawColor(renderer.renderer, 255, 255, 255, 255);
        SDL_SetRenderScale(renderer.renderer, 2.0f, 2.0f);
        SDL_RenderDebugTextFormat(renderer.renderer, 10.0f, 10.0f, "FPS: %.2f", currentFps);
        SDL_SetRenderScale(renderer.renderer, 1.0f, 1.0f);

        end_frame(&renderer);

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
    render_free_debugSquares_list(&debugSquaresList);
    textureManager_free_walls_list(&textures);
    objects_free_objects_list(&objectsList);

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}