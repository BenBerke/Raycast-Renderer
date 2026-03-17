//
// Created by berke on 3/16/2026.
//

#ifndef RAYCAST_RENDERER_APPSTATE_H
#define RAYCAST_RENDERER_APPSTATE_H
#include <SDL3/SDL_gpu.h>

#include "Systems/InputManager.h"

typedef struct {
    SDL_Window* window;
    SDL_GPUDevice* device;
    SDL_GPUGraphicsPipeline* pipeline;
    Uint32 numVertices;
    Uint32 numIndices;
    Uint32 numSlices;
    SDL_GPUBuffer* vertexBuffer;
    SDL_GPUBuffer* indexBuffer;
    SDL_GPUBuffer* rayStorageBuffer;
    InputManager inputManager;

    SDL_GPUGraphicsPipeline* debugPipeline;
    SDL_GPUBuffer* debugVertexBuffer;
    Uint32 debugVertexCapacity;
} AppState;

#endif //RAYCAST_RENDERER_APPSTATE_H