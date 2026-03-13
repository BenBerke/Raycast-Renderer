#include <math.h>
#include <SDL3/SDL.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"
#include "Headers/Systems/TextureManager.h"

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

    Texture skyBoxTexture = {0};
    Texture wallTexture = {0};

    if (!create_texture(&skyBoxTexture, "skybox", renderer.renderer)) {
        destroy_renderer(&renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!create_texture(&wallTexture, "wall", renderer.renderer)) {
        destroy_texture(&skyBoxTexture);
        destroy_renderer(&renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const float skyDstHeight = SCREEN_HEIGHT;
    const float skyDstWidth = skyBoxTexture.width * SCREEN_HEIGHT / skyBoxTexture.height;
    const SDL_FRect skyBoxRect = {0.0f, 0.0f, skyDstWidth, skyDstHeight};

    InputManager inputManager = {0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);

    DebugSquaresList debugSquaresList;
    render_create_debugSquares_list(&debugSquaresList, 8);

    Wall walls[] = {
        {{-260, 220}, {220, 40}, {255, 80, 80}},
        {{40, 220},   {180, 40}, {255, 170, 60}},
        {{280, 220},  {140, 40}, {255, 230, 90}},

        {{-320, 120}, {40, 160}, {80, 220, 120}},
        {{-220, 100}, {140, 40}, {70, 180, 255}},
        {{-120, 20},  {40, 120}, {90, 120, 255}},
        {{-260, -40}, {180, 40}, {140, 100, 255}},

        {{0, 120},    {40, 140}, {200, 90, 255}},
        {{80, 40},    {120, 40}, {255, 90, 200}},
        {{-20, -80},  {160, 40}, {255, 100, 140}},
        {{40, -180},  {40, 120}, {220, 220, 220}},

        {{220, 120},  {160, 40}, {100, 220, 100}},
        {{320, 20},   {40, 160}, {80, 220, 220}},
        {{220, -80},  {140, 40}, {80, 140, 255}},
        {{300, -180}, {120, 40}, {180, 80, 80}},

        {{-260, -220}, {180, 40}, {120, 80, 40}},
        {{0, -240},    {140, 40}, {60, 60, 60}},
        {{240, -240},  {180, 40}, {40, 120, 60}},

        {{-60, 180},   {40, 40}, {255, 255, 255}},
        {{160, 180},   {40, 40}, {255, 255, 255}},
        {{-180, -140}, {50, 50}, {255, 220, 120}},
        {{140, -20},   {50, 50}, {120, 255, 200}},
    };

    const int wallCount = (int)(sizeof(walls) / sizeof(walls[0]));
    for (int i = 0; i < wallCount; i++) {
        physics_push_walls_list(&wallsList, &walls[i]);
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

        begin_frame(&renderer, skyBoxTexture.texture, &skyBoxRect);

        renderer_draw_walls(
            &renderer,
            wallTexture.texture,
            wallTexture.width,
            wallTexture.height,
            &player,
            &wallsList,
            debug ? &debugSquaresList : NULL
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

    destroy_texture(&skyBoxTexture);
    destroy_texture(&wallTexture);
    physics_free_walls_list(&wallsList);
    render_free_debugSquares_list(&debugSquaresList);

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}