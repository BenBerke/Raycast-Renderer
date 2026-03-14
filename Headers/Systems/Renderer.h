#ifndef RAYCAST_RENDERER_RENDERER_H
#define RAYCAST_RENDERER_RENDERER_H

#include <SDL3/SDL.h>

#include "../Objects/Player.h"
#include "../Objects/DebugSquare.h"
#include "../Objects/Object.h"
#include "Physics.h"
#include "TextureManager.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} Renderer;

typedef struct {
    DebugSquare* items;
    int count;
    int size;
    int chunkSize;
} DebugSquaresList;

Renderer create_renderer(SDL_Window* window, SDL_Renderer* renderer);
void destroy_renderer(const Renderer* renderer);

void begin_frame(const Renderer* renderer, SDL_Texture* skyBox, const SDL_FRect* skyDst);
void end_frame(const Renderer* renderer);

void render_create_debugSquares_list(DebugSquaresList* list, int chunkSize);
void render_push_debugSquares_list(DebugSquaresList* list, DebugSquare* value);
void render_pop_debugSquares_list(DebugSquaresList* list);
void render_free_debugSquares_list(DebugSquaresList* list);

void render_debug_walls(const Renderer* renderer, const WallsList* walls);
void render_debug_player(const Renderer* renderer, const Player* player);
void render_debug_squares(const Renderer* renderer, const DebugSquaresList* squares);
void render_draw_grid_line(const Renderer* renderer);

void renderer_draw(
    const TexturesList* texturesList,
    const Player* player,
    const WallsList* walls,
    const DebugSquaresList* debugSquares,
    const Renderer* renderer,
    ObjectsList* objects
);

void renderer_draw_objects(ObjectsList* objectList, Player* p);

#endif