#include "../../Headers/Systems/Renderer.h"

#include <math.h>
#include <stdlib.h>

#include "../../config.h"
#include "../../Headers/Systems/Raycast.h"

Renderer create_renderer(SDL_Window* window, SDL_Renderer* renderer) {
    Renderer r;
    r.window = window;
    r.renderer = renderer;
    return r;
}

void destroy_renderer(const Renderer* renderer) {
    SDL_DestroyRenderer(renderer->renderer);
}

void begin_frame(const Renderer* renderer, SDL_Texture* skyBox, const SDL_FRect* skyDst) {
    SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer->renderer);

    if (skyBox != NULL) {
        SDL_RenderTexture(renderer->renderer, skyBox, NULL, skyDst);
    }
}

void end_frame(const Renderer* renderer) {
    SDL_RenderPresent(renderer->renderer);
}

void render_create_debugSquares_list(DebugSquaresList* list, int chunkSize) {
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc((size_t)list->size * sizeof(*list->items));
}

void render_push_debugSquares_list(DebugSquaresList* list, DebugSquare* value) {
    if (++list->count > list->size) {
        list->size += list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
    list->items[list->count - 1] = *value;
}

void render_pop_debugSquares_list(DebugSquaresList* list) {
    if (list->count <= 0) {
        return;
    }

    list->count--;

    if (list->size - list->count >= list->chunkSize && list->size > list->chunkSize) {
        list->size -= list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
}

void render_free_debugSquares_list(DebugSquaresList* list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->size = 0;
    list->chunkSize = 0;
}

void render_debug_walls(const Renderer* renderer, const WallsList* walls) {
    for (int i = 0; i < walls->count; i++) {
        SDL_SetRenderDrawColor(
            renderer->renderer,
            (Uint8)walls->items[i].color.x,
            (Uint8)walls->items[i].color.y,
            (Uint8)walls->items[i].color.z,
            255
        );

        const float x = walls->items[i].position.x;
        const float y = walls->items[i].position.y;
        const float w = walls->items[i].scale.x;
        const float h = walls->items[i].scale.y;

        const float screenX = (x + SCREEN_WIDTH / 2.0f) - w / 2.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - h / 2.0f;

        SDL_FRect rect = {screenX, screenY, w, h};
        SDL_RenderFillRect(renderer->renderer, &rect);
    }
}

void render_debug_player(const Renderer* renderer, const Player* player) {
    SDL_SetRenderDrawColor(renderer->renderer, 160, 160, 60, 255);

    const float x = player->position.x;
    const float y = player->position.y;
    const float a = player->scale;

    const float screenX = (x + SCREEN_WIDTH / 2.0f) - a / 2.0f;
    const float screenY = (SCREEN_HEIGHT / 2.0f - y) - a / 2.0f;

    SDL_FRect rect = {screenX, screenY, a, a};
    SDL_RenderFillRect(renderer->renderer, &rect);
}

void render_debug_squares(const Renderer* renderer, const DebugSquaresList* squares) {
    for (int i = 0; i < squares->count; i++) {
        SDL_SetRenderDrawColor(
            renderer->renderer,
            (Uint8)squares->items[i].color.x,
            (Uint8)squares->items[i].color.y,
            (Uint8)squares->items[i].color.z,
            255
        );

        const float x = squares->items[i].position.x;
        const float y = squares->items[i].position.y;
        const float w = squares->items[i].scale.x;
        const float h = squares->items[i].scale.y;

        const float screenX = (x + SCREEN_WIDTH / 2.0f) - w / 2.0f;
        const float screenY = (SCREEN_HEIGHT / 2.0f - y) - h / 2.0f;

        SDL_FRect rect = {screenX, screenY, w, h};
        SDL_RenderFillRect(renderer->renderer, &rect);
    }
}

void render_draw_grid_line(const Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer->renderer, 45, 45, 45, 255);
    SDL_RenderLine(renderer->renderer, SCREEN_WIDTH / 2.0f, 0.0f, SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT);
    SDL_RenderLine(renderer->renderer, 0.0f, SCREEN_HEIGHT / 2.0f, SCREEN_WIDTH, SCREEN_HEIGHT / 2.0f);
}

void renderer_draw_walls(
    const Renderer* renderer,
    SDL_Texture* wallTexture,
    float textureWidth,
    float textureHeight,
    const Player* player,
    const WallsList* walls,
    DebugSquaresList* debugSquares
) {
    const float fovRadians = FOV * ((float)M_PI / 180.0f);
    const float step = fovRadians / (float)(RAY_COUNT - 1);
    const float projectionPlane = (SCREEN_WIDTH / 2.0f) / tanf(fovRadians / 2.0f);
    const float wallWorldHeight = 100.0f;
    const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;

    for (int i = 0; i < RAY_COUNT; i++) {
        Ray ray = {{player->position.x, player->position.y}};
        const float rayAngle = player->angle + fovRadians / 2.0f - (float)i * step;
        const Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

        const RayReturn hit = raycast_create_ray(&ray, player, dir, walls);
        if (hit.distance < 0.0f) {
            continue;
        }

        float correctedDistance = hit.distance * cosf(rayAngle - player->angle);
        if (correctedDistance < 0.001f) {
            correctedDistance = 0.001f;
        }

        const float wallHeight = (wallWorldHeight / correctedDistance) * projectionPlane;

        int xStart = (int)floorf((float)i * sliceWidth);
        int xEnd = (int)ceilf((float)(i + 1) * sliceWidth);
        if (xEnd <= xStart) {
            xEnd = xStart + 1;
        }

        const float y1 = SCREEN_HEIGHT / 2.0f - wallHeight / 2.0f;
        const float y2 = SCREEN_HEIGHT / 2.0f + wallHeight / 2.0f;

        const float maxDist = 700.0f;
        const float ambient = 0.25f;

        float t = correctedDistance / maxDist;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        float brightness = ambient + (1.0f - ambient) * (1.0f - t);
        switch (hit.side) {
            case 0: break;
            case 1: brightness *= 0.30f; break;
            case 2:
            case 3: brightness *= 0.70f; break;
            default: break;
        }

        Uint8 r = (Uint8)(hit.r * brightness);
        Uint8 g = (Uint8)(hit.g * brightness);
        Uint8 b = (Uint8)(hit.b * brightness);

        int texX = (int)(hit.u * textureWidth);
        if (texX < 0) texX = 0;
        if (texX >= (int)textureWidth) texX = (int)textureWidth - 1;

        if (hit.side == 0 || hit.side == 2) {
            texX = (int)textureWidth - 1 - texX;
        }

        SDL_FRect src = { (float)texX, 0.0f, 1.0f, textureHeight };
        SDL_FRect dst = {
            (float)xStart,
            y1,
            (float)(xEnd - xStart),
            y2 - y1
        };

        SDL_SetTextureColorMod(wallTexture, r, g, b);
        SDL_RenderTexture(renderer->renderer, wallTexture, &src, &dst);

        if (debugSquares != NULL && i < debugSquares->count) {
            debugSquares->items[i].position = ray.position;
        }
    }
}