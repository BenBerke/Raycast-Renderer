
#include <math.h>
#include <stdlib.h>

#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"
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

void renderer_draw(
    const TexturesList* texturesList,
    const Player* player,
    const WallsList* walls,
    const DebugSquaresList* debugSquares,
    const Renderer* renderer,
    ObjectsList* objects
) {
    const float fovRadians = FOV * ((float)M_PI / 180.0f);
    const float step = fovRadians / (float)(RAY_COUNT - 1);
    const float projectionPlane = SCREEN_WIDTH / 2.0f / tanf(fovRadians / 2.0f);
    const float sliceWidth = (float)SCREEN_WIDTH / (float)RAY_COUNT;
    const float wallWorldHeight = WALL_HEIGHT;

    float columnDepthBuffer[RAY_COUNT];
    static float columnWallDists[RAY_COUNT][MAX_WALL_OVERLAP];
    static float columnWallTops[RAY_COUNT][MAX_WALL_OVERLAP];
    static int columnWallCounts[RAY_COUNT];

    for (int i = 0; i < RAY_COUNT; i++) {
        columnDepthBuffer[i]  = 1e9f;
        columnWallCounts[i]   = 0;
    }

    for (int rayIndex = 0; rayIndex < RAY_COUNT; rayIndex++) {
        Ray nearestRay = {{player->position.x, player->position.y}};
        const float rayAngle = player->angle + fovRadians / 2.0f - (float)rayIndex * step;
        const Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

        RayReturn hits[MAX_WALL_OVERLAP];
        const int hitCount = raycast_collect_hits(
            &nearestRay,
            player,
            dir,
            walls,
            hits,
            MAX_WALL_OVERLAP
        );

        if (hitCount <= 0) {
            continue;
        }

        const int xStart = (int)floorf((float)rayIndex * sliceWidth);
        int xEnd = (int)ceilf((float)(rayIndex + 1) * sliceWidth);
        if (xEnd <= xStart) {
            xEnd = xStart + 1;
        }

        for (int hitIndex = 0; hitIndex < hitCount; hitIndex++) {
            const RayReturn hit = hits[hitIndex];

            float correctedDistance = hit.distance * cosf(rayAngle - player->angle);
            if (correctedDistance < .5f) {
                continue;
            }

            const float wallHeight = (wallWorldHeight / correctedDistance) * projectionPlane;

            const float horizon = SCREEN_HEIGHT / 2.0f;
            const float y2 = horizon + wallHeight * 0.5f;
            const float y1 = y2 - wallHeight * hit.height;

            const float maxDist = 700.0f;
            const float ambient = 0.4f;

            float t = correctedDistance / maxDist;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float brightness = ambient + (1.0f - ambient) * (1.0f - t);
            switch (hit.side) {
                case 0:
                    break;
                case 1:
                    brightness *= 0.30f;
                    break;
                case 2:
                case 3:
                    brightness *= 0.70f;
                    break;
                default:
                    break;
            }

            Uint8 r = (Uint8)(hit.r * brightness);
            Uint8 g = (Uint8)(hit.g * brightness);
            Uint8 b = (Uint8)(hit.b * brightness);

            if (hit.texture < 0 || hit.texture >= texturesList->count) {
                continue;
            }

            const float textureWidth = texturesList->items[hit.texture].width;
            const float textureHeight = texturesList->items[hit.texture].height;
            SDL_Texture* wallTexture = texturesList->items[hit.texture].texture;

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

            if (hit.distance < columnDepthBuffer[rayIndex]) {
                columnDepthBuffer[rayIndex] = hit.distance;
            }
            if (columnWallCounts[rayIndex] < MAX_WALL_OVERLAP) {
                int idx = columnWallCounts[rayIndex]++;
                columnWallDists[rayIndex][idx] = hit.distance;
                columnWallTops[rayIndex][idx]  = y1;
            }
        }

        if (debugSquares != NULL && rayIndex < debugSquares->count) {
            debugSquares->items[rayIndex].position = nearestRay.position;
        }
    }
    for (int a = 0; a < objects->count - 1; a++) {
        for (int b = a + 1; b < objects->count; b++) {
            float distA = vector2_distance(player->position, objects->items[a].position);
            float distB = vector2_distance(player->position, objects->items[b].position);
            if (distB > distA) {
                Object tmp = objects->items[a];
                objects->items[a] = objects->items[b];
                objects->items[b] = tmp;
            }
        }
    }
    for (int i = 0; i < objects->count; i++) {
        const Object currentObject = objects->items[i];
        const float distance = vector2_distance(player->position, currentObject.position);
        float dx = currentObject.position.x - player->position.x;
        float dy = currentObject.position.y - player->position.y;
        float objectAngle = atan2f(dy, dx);
        float relativeAngle = objectAngle - player->angle;

        while (relativeAngle <= -M_PI) relativeAngle += 2 * M_PI;
        while (relativeAngle > M_PI)  relativeAngle -= 2 * M_PI;

        float screenX = (SCREEN_WIDTH / 2.0f) * (1.0f - tanf(relativeAngle) / tanf(fovRadians / 2.0f));
        float correctedDistance = distance * cosf(relativeAngle);

        float baseHeight    = (WALL_HEIGHT / correctedDistance) * projectionPlane;
        float spriteHeight  = baseHeight * currentObject.scale.y;

        float spriteBottom  = (SCREEN_HEIGHT / 2.0f) + baseHeight / 2.0f; // always fixed
        float spriteTop     = spriteBottom - spriteHeight;

        if (currentObject.texture < 0 || currentObject.texture >= texturesList->count) {
            continue;
        }

        SDL_Texture* spriteTexture = texturesList->items[currentObject.texture].texture;
        if (spriteTexture == NULL) continue;

        float texWidth  = texturesList->items[currentObject.texture].width;
        float texHeight = texturesList->items[currentObject.texture].height;

        float spriteWidth = spriteHeight * (texWidth / texHeight) * currentObject.scale.x;
        float spriteLeft  = screenX - spriteWidth / 2.0f;

        int colStart = (int)floorf(spriteLeft);
        int colEnd   = (int)ceilf(spriteLeft + spriteWidth);

        const float maxDist = 700.0f;
        const float ambient = 0.4f;
        float fade = correctedDistance / maxDist;
        if (fade < 0.0f) fade = 0.0f;
        if (fade > 1.0f) fade = 1.0f;

        float brightnessF = ambient + (1.0f - ambient) * (1.0f - fade);
        Uint8 r = (Uint8)(currentObject.color.x * brightnessF);
        Uint8 g = (Uint8)(currentObject.color.y * brightnessF);
        Uint8 b = (Uint8)(currentObject.color.z * brightnessF);
        SDL_SetTextureColorMod(spriteTexture, r, g, b);

        for (int col = colStart; col < colEnd; col++) {
            if (col < 0 || col >= SCREEN_WIDTH) continue;

            int bufferIndex = (int)((float)col / ((float)SCREEN_WIDTH / (float)RAY_COUNT));
            if (bufferIndex < 0) bufferIndex = 0;
            if (bufferIndex >= RAY_COUNT) bufferIndex = RAY_COUNT - 1;

            // Find highest wall top (min Y) among walls CLOSER than this sprite
            float clipTop = SCREEN_HEIGHT;
            for (int w = 0; w < columnWallCounts[bufferIndex]; w++) {
                if (columnWallDists[bufferIndex][w] < correctedDistance) {
                    if (columnWallTops[bufferIndex][w] < clipTop) {
                        clipTop = columnWallTops[bufferIndex][w];
                    }
                }
            }

            float dstTop    = spriteTop;
            float dstBottom = spriteTop + spriteHeight;
            if (dstBottom > clipTop) dstBottom = clipTop;
            if (dstTop >= dstBottom) continue;

            float srcYStart  = (dstTop - spriteTop) / spriteHeight * texHeight;
            float srcYHeight = (dstBottom - dstTop) / spriteHeight * texHeight;

            float t    = (float)(col - colStart) / spriteWidth;
            float srcX = t * texWidth;

            SDL_FRect src = { srcX, srcYStart, 1.0f, srcYHeight };
            SDL_FRect dst = { (float)col, dstTop, 1.0f, dstBottom - dstTop };

            SDL_RenderTexture(renderer->renderer, spriteTexture, &src, &dst);
        }
    }
}
