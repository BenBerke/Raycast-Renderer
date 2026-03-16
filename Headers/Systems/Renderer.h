#ifndef RAYCAST_RENDERER_RENDERER_H
#define RAYCAST_RENDERER_RENDERER_H

#include <SDL3/SDL_gpu.h>

#include "Raycast.h"
#include "../AppState.h"

typedef struct {
    float x, y, z;
} Vertex;

SDL_GPUShader* renderer_load_shader(SDL_GPUDevice* device, const char* fileName);
bool renderer_create_pipeline(AppState* state);
bool renderer_create_index_buffer(AppState* state, Uint16* indices, int indexCount);
bool renderer_create_vertex_buffer(AppState* state, Vertex* vertices, int vertexCount);
bool renderer_update_raySlice_data_buffer(AppState* state, const RaySlice* rayData, Uint32 count);

#endif