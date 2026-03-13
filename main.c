#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_timer.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"
#include "Headers/Systems/Raycast.h"

#include "Headers/Objects/Wall.h"

static Uint32 fpsTimer = 0;
static int frames = 0;

static char* assetsLocation = "Assets/";

typedef struct {
    SDL_Texture *texture;
    float width, height;
} Texture;

void create_texture(Texture* texture, const char* texturePath, const Renderer* renderer) {
    char fullPath[strlen(assetsLocation) + strlen(texturePath) + strlen(".png") + 1];
    snprintf(fullPath, sizeof(fullPath), "Assets/%s.png", texturePath);
    texture->texture = IMG_LoadTexture(renderer->renderer, fullPath);
    if (!texture->texture) {
        SDL_Log("IMG_LoadTexture %s failed: %s", texturePath, SDL_GetError());
        return;
    }
    SDL_GetTextureSize(texture->texture,&texture->width, &texture->height);
}

void movement(const InputManager* inputManager, Player* p) {
    const Vector2 forward = { cosf(p->angle), sinf(p->angle) };
    const Vector2 right = { -sinf(p->angle), cosf(p->angle) };

    if (input_manager_get_key(inputManager, SDL_SCANCODE_W)) {
        player_add_velocity(p, vector2_multiply_with_float(forward, p->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_S)) {
        player_add_velocity(p, vector2_multiply_with_float(forward, -p->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_A)) {
        player_add_velocity(p, vector2_multiply_with_float(right, p->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_D)) {
        player_add_velocity(p, vector2_multiply_with_float(right, -p->speed));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_RIGHT) || input_manager_get_key(inputManager, SDL_SCANCODE_E)) {
        p->angle -= PLAYER_ROT_SPEED;
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_LEFT) || input_manager_get_key(inputManager, SDL_SCANCODE_Q)) {
        p->angle += PLAYER_ROT_SPEED;
    }
}


int initialize(SDL_Window** window, SDL_Renderer** renderer) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create the window and assign it directly to the dereferenced pointer
    *window = SDL_CreateWindow("SDL3 Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!*window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Pass the actual window to the renderer creation
    *renderer = SDL_CreateRenderer(*window, NULL);
    if (!*renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 1;
    }

    return 0; // Success
}

int main(){
    bool debug = false;

    SDL_Renderer* _renderer = 0;
    SDL_Window* window = 0;
    if (initialize(&window, &_renderer)) return 1;

    Renderer renderer = create_renderer(window, _renderer);

    Texture skyBoxTexture = {0, 0, 0};
    create_texture(&skyBoxTexture, "skybox", &renderer);
    Texture wallTexture = {0, 0, 0};
    create_texture(&wallTexture, "wall", &renderer);


    const float targetH = SCREEN_HEIGHT;
    const float targetW = skyBoxTexture.width *  SCREEN_HEIGHT / skyBoxTexture.height;
    SDL_FRect skyBoxRect = {0, 0, targetW, targetH };

    // ------------------------------------------------------------//
    // ---------------------GAME-----------------------------------//
    //-------------------------------------------------------------//

    InputManager inputManager = {0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);

    DebugSquaresList debugSquaresList;
    render_create_debugSquares_list(&debugSquaresList, 4);

    Wall walls[] = {
        // ----- TOP SECTION -----
        {{-260, 220}, {220, 40}, {255, 80, 80}},
        {{40, 220},   {180, 40}, {255, 170, 60}},
        {{280, 220},  {140, 40}, {255, 230, 90}},

        // ----- LEFT ROOMS / CORRIDORS -----
        {{-320, 120}, {40, 160}, {80, 220, 120}},
        {{-220, 100}, {140, 40}, {70, 180, 255}},
        {{-120, 20},  {40, 120}, {90, 120, 255}},
        {{-260, -40}, {180, 40}, {140, 100, 255}},

        // ----- CENTRE STRUCTURE -----
        {{0, 120},    {40, 140}, {200, 90, 255}},
        {{80, 40},    {120, 40}, {255, 90, 200}},
        {{-20, -80},  {160, 40}, {255, 100, 140}},
        {{40, -180},  {40, 120}, {220, 220, 220}},

        // ----- RIGHT ROOMS / CORRIDORS -----
        {{220, 120},  {160, 40}, {100, 220, 100}},
        {{320, 20},   {40, 160}, {80, 220, 220}},
        {{220, -80},  {140, 40}, {80, 140, 255}},
        {{300, -180}, {120, 40}, {180, 80, 80}},

        // ----- LOWER SECTION -----
        {{-260, -220},{180, 40}, {120, 80, 40}},
        {{0, -240},   {140, 40}, {60, 60, 60}},
        {{240, -240}, {180, 40}, {40, 120, 60}},

        // ----- SMALL LANDMARK BLOCKS -----
        {{-60, 180},  {40, 40},  {255, 255, 255}},
        {{160, 180},  {40, 40},  {255, 255, 255}},
        {{-180, -140},{50, 50},  {255, 220, 120}},
        {{140, -20},  {50, 50},  {120, 255, 200}},
    };

    for (int i = 0; i < sizeof(walls)/sizeof(walls[0]); i++) physics_push_walls_list(&wallsList, &walls[i]);
    for (int i = 0; i < RAY_COUNT; i++) {
        DebugSquare ds = {{0,0}, {20, 20}, {0, 0, 255}};
        render_push_debugSquares_list(&debugSquaresList, &ds);
    }

    Player p = {{0, 0}, PLAYER_SCALE, {0, 0}, PLAYER_SPEED, PLAYER_FRICTION, 0};

    bool running = true;
    while (running) {
        Uint32 startTime = SDL_GetTicks();

        //--------------------------------//
        //--------------INPUT-------------//
        //--------------------------------//
        input_manager_begin_frame(&inputManager);

        if (input_manager_get_key_down(&inputManager,SDL_SCANCODE_ESCAPE)) running = false;

        movement(&inputManager, &p);
        debug = input_manager_get_key(&inputManager, SDL_SCANCODE_TAB);

        //--------------------------------//
        //--------------------------------//
        //--------------------------------//

        player_update(&p);
        physics_check_collisions(&p, &wallsList);

        begin_frame(&renderer, skyBoxTexture.texture, &skyBoxRect);

        renderer_draw_wall(wallTexture.texture, wallTexture.width, wallTexture.height, &renderer, &p, &wallsList);

        if (debug) {
            render_debug_walls(&renderer, &wallsList);
            render_debug_player(&renderer, &p);
            render_debug_squares(&renderer, &debugSquaresList);
        }


        Uint32 frameTime = SDL_GetTicks() - startTime;
        if (FRAME_DELAY > frameTime) SDL_Delay(FRAME_DELAY - frameTime);

        frames++;

        float fps;
        Uint32 now = SDL_GetTicks();
        if (now - fpsTimer >= 1000) {
            fps = frames * 1000.0f / (float)(now - fpsTimer);
            fpsTimer = now;
            frames = 0;
        }
        SDL_SetRenderDrawColor(renderer.renderer, 255, 255, 255, 255);
        SDL_SetRenderScale(renderer.renderer, 2.0f, 2.0f);
        SDL_RenderDebugTextFormat(renderer.renderer, 20, 20, "FPS: %.2f", fps);
        SDL_SetRenderScale(renderer.renderer, 1.0f, 1.0f);
        end_frame(&renderer);
    }

    physics_free_walls_list(&wallsList);
    render_free_debugSquares_list(&debugSquaresList);

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
