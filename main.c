#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include "config.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Physics.h"

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

    InputManager inputManager;

    Wall w = {{120, 120}, {60, 60}, {0, 0, 0}};
    Player p = {{0, 0}, 60, {0, 0}, 6, 0.5f};

    bool running = true;
    while (running) {
        Uint32 startTime = SDL_GetTicks();
        input_manager_begin_frame(&inputManager);
        player_update(&p);
        physics_check_collisions(&p, &w);

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

        begin_frame(&renderer);

        render_wall(&renderer, &w);
        render_player(&renderer, &p);

        end_frame(&renderer);

        Uint32 endTime = SDL_GetTicks() - startTime;
        if (FRAME_DELAY > endTime) SDL_Delay(FRAME_DELAY - endTime);
    }

    destroy_renderer(&renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}