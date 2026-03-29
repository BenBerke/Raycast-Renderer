#pragma once

#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "Raycast.h"

typedef struct {
    float x, y, z;
    float u, v;
} Vertex;

typedef struct {
    SDL_GPUDevice*            device;
    SDL_GPUBuffer*            rayBuffer;
    SDL_GPUTransferBuffer*    rayTransferBuffer;
    SDL_GPUBuffer*            vertexBuffer;
    SDL_GPUTransferBuffer*    vertexTransferBuffer;
    SDL_GPUGraphicsPipeline*  graphicsPipeline;
} Renderer;

/* Initialise every GPU resource. Returns false on failure. */
bool renderer_init(Renderer* renderer, SDL_Window* window);

/*
 * Upload cpuSlices, acquire a swapchain texture and draw one frame.
 * Returns false when the swapchain texture was unavailable (skip frame).
 */
bool renderer_render_frame(Renderer* renderer, SDL_Window* window, RaySlice* cpuSlices);

/* Release all GPU resources owned by the renderer. */
void renderer_destroy(Renderer* renderer);