//
// Created by berke on 3/11/2026.
//

#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"

#include <math.h>

#include "../../config.h"

Renderer create_renderer(SDL_Window *window, SDL_Renderer *renderer) {
    Renderer r;
    r.window = window;
    r.renderer = renderer;
    return r;
}

void begin_frame(const Renderer *renderer) {
    SDL_Renderer* r = renderer->renderer;
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderClear(r);
    render_draw_grid_line(renderer);
}

void end_frame(Renderer *renderer) {
    SDL_Renderer* r = renderer->renderer;
    SDL_RenderPresent(r);
}

void destroy_renderer(Renderer *renderer) {
    SDL_DestroyRenderer(renderer->renderer);
}

void render_wall(const Renderer *renderer, Wall* wall) {
    SDL_SetRenderDrawColor(renderer->renderer, (Uint8)wall->color.x, (Uint8)wall->color.y, (Uint8)wall->color.z, 255);

    float x = wall->position.x;
    float y = wall->position.y;

    float scaleX = wall->scale.x;
    float scaleY = wall->scale.y;

    float screenX = (x + SCREEN_WIDTH / 2) - scaleX/2;
    float screenY = (y + SCREEN_HEIGHT / 2) - scaleY/2;

    SDL_FRect rect = {screenX, screenY, scaleX, scaleY};
    SDL_RenderFillRect(renderer->renderer, &rect);
}

void render_player(const Renderer *renderer, const Player* player) {
    SDL_SetRenderDrawColor(renderer->renderer, 160, 160, 60, 255);
    float x = player->position.x, y = player->position.y;
    float a = player->scale;

    float screenX = (x + SCREEN_WIDTH / 2) - a/2;
    float screenY = (y + SCREEN_HEIGHT / 2) - a/2;

    SDL_FRect rect = {screenX, screenY, a, a};
    SDL_RenderFillRect(renderer->renderer, &rect);

    // const SDL_FPoint cornerA = {screenX, screenY};
    // const SDL_FPoint cornerB = {screenX + a, screenY};
    // const SDL_FPoint cornerC = {screenX + a/2, (float)(screenY - a * sqrt(3)/2)};
    //
    // SDL_FPoint corners[] = {cornerA, cornerB, cornerC, cornerA};
    // SDL_RenderLines(renderer->renderer, corners, sizeof(corners)/sizeof(SDL_FPoint));
}

void render_draw_grid_line(const Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer->renderer, 45, 45, 45, 255);

    SDL_RenderLine(renderer->renderer, SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT);
    SDL_RenderLine(renderer->renderer, 0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2);
}