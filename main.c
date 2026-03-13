#include <math.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"
#include "Headers/Systems/Raycast.h"

#include "Headers/Objects/Wall.h"

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
    if (input_manager_get_key(inputManager, SDL_SCANCODE_Q)) {
        p->angle -= .08f;
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_E)) {
        p->angle += .08f;
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

int main(int argc, char *argv[]){
    bool debug = false;

    SDL_Renderer* _renderer = 0;
    SDL_Window* window = 0;
    if (initialize(&window, &_renderer)) return 1;

    Renderer renderer = create_renderer(window, _renderer);

    SDL_Texture *skyBoxTexture = IMG_LoadTexture(renderer.renderer, "Assets/skybox.png");
    if (!skyBoxTexture) SDL_Log("IMG_LoadTexture failed: %s", SDL_GetError());
    float texW, texH;
    SDL_GetTextureSize(skyBoxTexture, &texW, &texH);

    const float targetH = SCREEN_HEIGHT;
    const float targetW = texW *  SCREEN_HEIGHT / texH;

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
        // ----- OUTER BOUNDARY AT SCREEN EDGES -----
        //{{0, SCREEN_HEIGHT / 2.0f - 10},   {SCREEN_WIDTH, 20}, {0, 0, 0}},   // top
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

        begin_frame(&renderer, skyBoxTexture, &skyBoxRect);

        const float fov_in_rads = FOV * (M_PI / 180.0f);

        const float step = fov_in_rads / (float)(RAY_COUNT - 1);
        const float projectionPlane = (SCREEN_WIDTH / 2.0f) / tanf(fov_in_rads / 2.0f);
        const float wallWorldHeight = 100.0f;
        const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;

        for (int i = 0; i < RAY_COUNT; i++) {
            Ray r = {{p.position.x, p.position.y}};

            float rayAngle = p.angle + fov_in_rads / 2.0f - (float)i * step;
            Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

            float distance = raycast_create_ray(&r, &p, dir, &wallsList);
            float correctedDistance = 0;
            float wallHeight = 0;
            if (distance != -1) {
                correctedDistance = distance * cosf(rayAngle - p.angle);
                if (correctedDistance < 0.001f) correctedDistance = 0.001f;

                wallHeight = (wallWorldHeight / correctedDistance) * projectionPlane;
            }

            int xStart = (int)floorf((float)i * sliceWidth);
            int xEnd = (int)ceilf((float)(i + 1) * sliceWidth);

            if (xEnd <= xStart) xEnd = xStart + 1;

            float y1 = SCREEN_HEIGHT / 2.0f - wallHeight / 2.0f;
            float y2 = SCREEN_HEIGHT / 2.0f + wallHeight / 2.0f;

            float brightness = 255.0f - correctedDistance * 0.8f;
            if (brightness < 100.0f) brightness = 100.0f;
            if (brightness > 255.0f) brightness = 255.0f;

            Uint8 shade = (Uint8)brightness;

            SDL_SetRenderDrawColor(renderer.renderer, 0, 0, shade, 255);

            SDL_FRect slice = {
                (float)xStart,
                y1,
                (float)(xEnd - xStart),
                y2 - y1
            };

            SDL_RenderFillRect(renderer.renderer, &slice);

            if (debug) debugSquare_set_position(&debugSquaresList.items[i], r.position);
        }

        if (debug) {
            render_walls(&renderer, &wallsList);
            render_player(&renderer, &p);
            render_debugSquares(&renderer, &debugSquaresList);
        }

        end_frame(&renderer);

        Uint32 endTime = SDL_GetTicks() - startTime;
        if (FRAME_DELAY > endTime) SDL_Delay(FRAME_DELAY - endTime);
    }

    physics_free_walls_list(&wallsList);
    render_free_debugSquares_list(&debugSquaresList);

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
