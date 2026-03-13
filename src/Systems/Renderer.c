//
// Created by berke on 3/11/2026.
//

#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"

#include <math.h>
#include <stdlib.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_surface.h>

void render_create_debugSquares_list(DebugSquaresList* list, int chunkSize) {
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc(list->chunkSize * sizeof(*(list->items)));
}
void render_push_debugSquares_list(DebugSquaresList* list, DebugSquare* value) {
    if(++list->count > list->size){
        list->items = realloc(list->items, (list->size+=list->chunkSize)*sizeof(*(list->items)));
    }
    list->items[list->count-1] = *value;
}
void render_pop_debugSquares_list(DebugSquaresList* list) {
    if(list->size - (--list->count) >= list->chunkSize && list->size > list->chunkSize){
        list->size -= list->chunkSize;
        list->items = realloc(list->items, list->size*sizeof(*(list->items)));
    }
}
void render_free_debugSquares_list(DebugSquaresList* list) {
    free(list->items);
}

Renderer create_renderer(SDL_Window *window, SDL_Renderer *renderer) {
    Renderer r;
    r.window = window;
    r.renderer = renderer;
    return r;
}

void begin_frame(const Renderer *renderer, SDL_Texture* skyBox, const SDL_FRect* dst) {
    SDL_Renderer* r = renderer->renderer;
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    SDL_RenderClear(r);

    SDL_RenderTexture(renderer->renderer, skyBox, NULL, dst);
    //render_draw_grid_line(renderer);
}

void end_frame(const Renderer *renderer) {
    SDL_Renderer* r = renderer->renderer;
    SDL_RenderPresent(r);
}

void destroy_renderer(const Renderer *renderer) {
    SDL_DestroyRenderer(renderer->renderer);
}

void render_walls(const Renderer *renderer, const WallsList* walls) {
    for (int i = 0; i < walls->count; i++) {

        SDL_SetRenderDrawColor(renderer->renderer, (Uint8)walls->items[i].color.x, (Uint8)walls->items[i].color.y, (Uint8)walls->items[i].color.z, 255);

        const float x = walls->items[i].position.x;
        const float y = walls->items[i].position.y;

        const float scaleX = walls->items[i].scale.x;
        const float scaleY = walls->items[i].scale.y;

        float screenX = (x + SCREEN_WIDTH / 2) - scaleX/2;
        float screenY = (SCREEN_HEIGHT / 2 - y) - scaleY/2;

        SDL_FRect rect = {screenX, screenY, scaleX, scaleY};
        SDL_RenderFillRect(renderer->renderer, &rect);
    }
}

void render_player(const Renderer *renderer, const Player* player) {
    SDL_SetRenderDrawColor(renderer->renderer, 160, 160, 60, 255);
    float x = player->position.x, y = player->position.y;
    float a = player->scale;

    float screenX = (x + SCREEN_WIDTH / 2) - a/2;
    float screenY = (SCREEN_HEIGHT / 2 - y) - a/2;

    SDL_FRect rect = {screenX, screenY, a, a};
    SDL_RenderFillRect(renderer->renderer, &rect);

    // const SDL_FPoint cornerA = {screenX, screenY};
    // const SDL_FPoint cornerB = {screenX + a, screenY};
    // const SDL_FPoint cornerC = {screenX + a/2, (float)(screenY - a * sqrt(3)/2)};
    //
    // SDL_FPoint corners[] = {cornerA, cornerB, cornerC, cornerA};
    // SDL_RenderLines(renderer->renderer, corners, sizeof(corners)/sizeof(SDL_FPoint));
}

void render_debugSquares(Renderer* renderer, const DebugSquaresList* squares) {
    for (int i = 0; i < squares->count; i++) {

        SDL_SetRenderDrawColor(renderer->renderer, (Uint8)squares->items[i].color.x, (Uint8)squares->items[i].color.y, (Uint8)squares->items[i].color.z, 255);

        const float x = squares->items[i].position.x;
        const float y = squares->items[i].position.y;

        const float scaleX = squares->items[i].scale.x;
        const float scaleY = squares->items[i].scale.y;

        float screenX = (x + SCREEN_WIDTH / 2) - scaleX/2;
        float screenY = (SCREEN_HEIGHT / 2 - y) - scaleY/2;

        SDL_FRect rect = {screenX, screenY, scaleX, scaleY};
        SDL_RenderFillRect(renderer->renderer, &rect);
    }
}

void render_draw_grid_line(const Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer->renderer, 45, 45, 45, 255);

    SDL_RenderLine(renderer->renderer, SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT);
    SDL_RenderLine(renderer->renderer, 0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2);
}


