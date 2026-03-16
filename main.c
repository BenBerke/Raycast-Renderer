#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_main.h>

#include <string.h>

#include "config.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Renderer.h"
#include "Headers/AppState.h"
#include "Headers/Systems/Raycast.h"

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    AppState state = {0};
    state.window = SDL_CreateWindow("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    state.device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL, true, "direct3d12");
    if (!state.device) {
        SDL_Log("Failed to create GPU device: %s", SDL_GetError());
        return -1;
    }

    if (!SDL_ClaimWindowForGPUDevice(state.device, state.window)) {
        SDL_Log("Failed to claim window for GPU device: %s", SDL_GetError());
        return -1;
    }

    if (!renderer_create_pipeline(&state)) return -1;
    Vertex vertices[] = {
        {-0.5f,  0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.5f,  0.5f, 0.0f},
    };

    Uint16 indices[] = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };
    renderer_create_vertex_buffer(&state, vertices, sizeof(vertices)/sizeof(vertices[0]));
    renderer_create_index_buffer(&state, indices, sizeof(indices)/sizeof(indices[0]));

    RaySlice rayData[RAY_COUNT] = {0};


    Player player = {.position = {0, 0}, .scale = 15.0f, .velocity = {0, 0}, .speed = 15.0f, .friction = 0.0f, .angle = 0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);
    Wall walls[] = {
        {.position = {100, 0}, .scale=15.0f, .color={255,255,255,255}, .textures={0,0,0,0}, .height=150, .faceBrightness = {0,0,0,0}},
    };
    for (int i = 0; i < sizeof(walls) / sizeof(walls[0]); i++) {
        physics_push_walls_list(&wallsList , &walls[i]);
    }

    raycast_to_gpu_buffer(&player, &wallsList, rayData);
    if (!renderer_create_raySlice_data_buffer(&state, rayData, RAY_COUNT)) {
        SDL_Log("Couldn't create ray slice data buffer");
        return -1;
    }

    bool running = true;
    while (running) {
        input_manager_begin_frame(&state.inputManager);
        if (input_manager_get_key_down(&state.inputManager, SDL_SCANCODE_ESCAPE)) running = false;
        if (input_manager_get_key_down(&state.inputManager, SDL_SCANCODE_W))

        if (!renderer_update_raySlice_data_buffer(&state, rayData, RAY_COUNT)) {
            SDL_Log("Couldn't update ray slice data buffer");
        }

        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(state.device);
        SDL_GPUTexture* swapChainTexture;
        if (SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, state.window, &swapChainTexture, NULL, NULL)) {
            if (swapChainTexture != NULL) {
                SDL_GPUColorTargetInfo colorTargetInfo = {
                    .texture = swapChainTexture,
                    .clear_color = (SDL_FColor){1.0f, 0.4f, 1.0f, 1.0f},
                    .load_op = SDL_GPU_LOADOP_CLEAR,
                    .store_op = SDL_GPU_STOREOP_STORE,
                };
                SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

                SDL_BindGPUGraphicsPipeline(renderPass, state.pipeline);
                SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){.buffer = state.vertexBuffer}, 1);
                SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){.buffer = state.indexBuffer}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
                SDL_BindGPUVertexStorageBuffers(renderPass,0, &state.rayStorageBuffer, 1);
                SDL_DrawGPUIndexedPrimitives(renderPass, state.numIndices, RAY_COUNT, 0, 0, 0);

                SDL_EndGPURenderPass(renderPass);
            }
        }
        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }

    physics_free_walls_list(&wallsList);
    SDL_ReleaseWindowFromGPUDevice(state.device, state.window);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
    return 0;
}
