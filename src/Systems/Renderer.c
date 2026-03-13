//
// Created by berke on 3/11/2026.
//

#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"
#include "../../Headers/Systems/Raycast.h"

#include <math.h>
#include <stdlib.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>

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

void render_debug_walls(const Renderer *renderer, const WallsList* walls) {
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

void render_debug_player(const Renderer *renderer, const Player* player) {
    SDL_SetRenderDrawColor(renderer->renderer, 160, 160, 60, 255);
    float x = player->position.x, y = player->position.y;
    float a = player->scale;

    float screenX = (x + SCREEN_WIDTH / 2) - a/2;
    float screenY = (SCREEN_HEIGHT / 2 - y) - a/2;

    SDL_FRect rect = {screenX, screenY, a, a};
    SDL_RenderFillRect(renderer->renderer, &rect);
}

void render_debug_squares(Renderer* renderer, const DebugSquaresList* squares) {
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

void renderer_draw_wall(SDL_Texture* wallTexture, float textureWidth, float textureHeight, Renderer* renderer,  Player* p, const WallsList *wallsList) {
    const float fov_in_rads = FOV * (M_PI / 180.0f);

        const float step = fov_in_rads / (float)(RAY_COUNT - 1);
        const float projectionPlane = (SCREEN_WIDTH / 2.0f) / tanf(fov_in_rads / 2.0f);
        const float wallWorldHeight = 100.0f;
        const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;

        for (int i = 0; i < RAY_COUNT; i++) {
            Ray ray = {{p->position.x, p->position.y}};

            const float rayAngle = p->angle + fov_in_rads / 2.0f - (float)i * step;
            const Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

            const RayReturn rayReturn = raycast_create_ray(&ray, p, dir, wallsList);

            float correctedDistance = 0.0f;
            float wallHeight = 0.0f;

            if (rayReturn.distance != -1.0f) {
                correctedDistance = rayReturn.distance * cosf(rayAngle - p->angle);
                if (correctedDistance < 0.001f) correctedDistance = 0.001f;

                wallHeight = (wallWorldHeight / correctedDistance) * projectionPlane;
            }

            int xStart = (int)floorf((float)i * sliceWidth);
            int xEnd = (int)ceilf((float)(i + 1) * sliceWidth);
            if (xEnd <= xStart) xEnd = xStart + 1;

            float y1 = SCREEN_HEIGHT / 2.0f - wallHeight / 2.0f;
            float y2 = SCREEN_HEIGHT / 2.0f + wallHeight / 2.0f;

            float maxDist = 700.0f;
            float ambient = 0.25f;

            float t = correctedDistance / maxDist;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;


            float brightness = ambient + (1.0f - ambient) * (1.0f - t);
            switch (rayReturn.side) {
                case 0: break;
                case 1: brightness *= .3f; break;
                case 2: brightness *= .7f; break;
                case 3: brightness *= .7f; break;
                default: break;
            }

            Uint8 r = (Uint8)(rayReturn.r * brightness);
            Uint8 g = (Uint8)(rayReturn.g * brightness);
            Uint8 b = (Uint8)(rayReturn.b * brightness);

            SDL_FRect slice = {
                (float)xStart,
                y1,
                (float)(xEnd - xStart),
                y2 - y1
            };

            int texX = (int)(rayReturn.u * textureWidth);
            if (texX < 0) texX = 0;
            if (texX >= (int)textureWidth) texX = (int)textureWidth - 1;

            if (rayReturn.side == 0 || rayReturn.side == 2) {
                texX = (int)textureWidth - 1 - texX;
            }

            SDL_FRect src = {(float)texX, 0.0f, 1.0f, textureHeight };

            SDL_SetTextureColorMod(wallTexture, r, g, b);
            SDL_RenderTexture(renderer->renderer, wallTexture, &src, &slice);
        }
}


