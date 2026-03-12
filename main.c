#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"
#include "Headers/Systems/Raycast.h"

#include "Headers/Objects/Wall.h"

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* _renderer = SDL_CreateRenderer(window, NULL);
    if (!_renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    Renderer renderer = create_renderer(window, _renderer);

    SDL_Window* window1 = SDL_CreateWindow("SDL3 Renderer", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window1) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* _renderer1 = SDL_CreateRenderer(window1, NULL);
    if (!_renderer1) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    Renderer renderer1 = create_renderer(window1, _renderer1);

    InputManager inputManager = {};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);

    DebugSquaresList debugSquaresList;
    render_create_debugSquares_list(&debugSquaresList, 4);

    Wall walls[] = {
        // ----- OUTER BOUNDARY AT SCREEN EDGES -----
        {{0, SCREEN_HEIGHT / 2.0f - 10},   {SCREEN_WIDTH, 20}, {0, 0, 0}},   // top
        {{0, -SCREEN_HEIGHT / 2.0f + 10},  {SCREEN_WIDTH, 20}, {0, 0, 0}},   // bottom
        {{-SCREEN_WIDTH / 2.0f + 10, 0},   {20, SCREEN_HEIGHT}, {0, 0, 0}},  // left
        {{SCREEN_WIDTH / 2.0f - 10, 0},    {20, SCREEN_HEIGHT}, {0, 0, 0}},  // right

        // ----- INNER OBSTACLES -----
        {{-220, 180}, {100, 60}, {0, 0, 0}},
         {{40, 200},   {140, 40}, {0, 0, 0}},
         {{240, 150},  {80, 100}, {0, 0, 0}},

         {{-260, 40},  {60, 140}, {0, 0, 0}},
         {{180, 20},   {70, 160}, {0, 0, 0}},

         {{-180, -120},{140, 50}, {0, 0, 0}},
         {{60, -160},  {120, 70}, {0, 0, 0}},
         {{250, -190}, {90, 90},  {0, 0, 0}},

         {{-60, 90},   {50, 50},  {0, 0, 0}},
         {{-20, -210}, {60, 60},  {0, 0, 0}},
    };

    for (int i = 0; i < sizeof(walls)/sizeof(walls[0]); i++) physics_push_walls_list(&wallsList, &walls[i]);
    for (int i = 0; i < RAY_COUNT; i++) {
        DebugSquare ds = {{0,0}, {20, 20}, {0, 0, 255}};
        render_push_debugSquares_list(&debugSquaresList, &ds);
    }

    Player p = {{0, 0}, 60, {0, 0}, 6, 0.5f, 0};

    bool running = true;
    while (running) {
        Uint32 startTime = SDL_GetTicks();
        input_manager_begin_frame(&inputManager);

        if (input_manager_get_key_down(&inputManager,SDL_SCANCODE_ESCAPE)) running = false;

        if (input_manager_get_key(&inputManager, SDL_SCANCODE_W)) {
            Vector2 vel = {0, p.speed};
            player_add_velocity(&p, vel);
        }
        if (input_manager_get_key(&inputManager, SDL_SCANCODE_S)) {
            Vector2 vel = {0, -p.speed};
            player_add_velocity(&p, vel);
        }
        if (input_manager_get_key(&inputManager, SDL_SCANCODE_A)) {
            Vector2 vel = {-p.speed, 0};
            player_add_velocity(&p, vel);
        }
        if (input_manager_get_key(&inputManager, SDL_SCANCODE_D)) {
            Vector2 vel = {p.speed, 0};
            player_add_velocity(&p, vel);
        }
        if (input_manager_get_key(&inputManager, SDL_SCANCODE_Q)) {
            p.angle += .08f;
        }
        if (input_manager_get_key(&inputManager, SDL_SCANCODE_E)) {
            p.angle -= .08f;
        }

        player_update(&p);
        physics_check_collisions(&p, &wallsList);

        begin_frame(&renderer);
        begin_frame(&renderer1);

        const float fov_in_rads = FOV * (M_PI / 180.0f);
        const float step = fov_in_rads / (float)(RAY_COUNT - 1);
        const float projectionPlane = (SCREEN_WIDTH / 2.0f) / tanf(fov_in_rads / 2.0f);
        const float wallWorldHeight = 100.0f;
        const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;

        for (int i = 0; i < RAY_COUNT; i++) {
            Ray r = {{p.position.x, p.position.y}};

            float rayAngle = p.angle - fov_in_rads / 2.0f + (float)i * step;
            Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

            float distance = raycast_create_ray(&r, &p, dir, &wallsList);

            float correctedDistance = distance * cosf(rayAngle - p.angle);
            if (correctedDistance < 0.001f) correctedDistance = 0.001f;

            float wallHeight = (wallWorldHeight / correctedDistance) * projectionPlane;

            float x = i * sliceWidth;
            float y1 = SCREEN_HEIGHT / 2.0f - wallHeight / 2.0f;
            float y2 = SCREEN_HEIGHT / 2.0f + wallHeight / 2.0f;

            SDL_SetRenderDrawColor(renderer1.renderer, 0, 0, 255-distance, 255);

            for (int w = 0; w < (int)sliceWidth; w++) {
                SDL_RenderLine(renderer1.renderer, x + w, y1, x + w, y2);
            }

            debugSquare_set_position(&debugSquaresList.items[i], r.position);
        }
        render_walls(&renderer, &wallsList);
        render_player(&renderer, &p);
        render_debugSquares(&renderer, &debugSquaresList);

        end_frame(&renderer);
        end_frame(&renderer1);

        Uint32 endTime = SDL_GetTicks() - startTime;
        if (FRAME_DELAY > endTime) SDL_Delay(FRAME_DELAY - endTime);
    }

    physics_free_walls_list(&wallsList);
    render_free_debugSquares_list(&debugSquaresList);

    destroy_renderer(&renderer);
    destroy_renderer(&renderer1);
    SDL_DestroyWindow(window);
    SDL_DestroyWindow(window1);
    SDL_Quit();

    return 0;
}