#include <math.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>

#include <string.h>

#include "config.h"
#include "Headers/Systems/InputManager.h"
#include "Headers/Systems/Raycast.h"

static Uint32 fpsTimer = 0;
static int frames = 0;
static float currentFps = 0.0f;

typedef struct {
    float x, y, z;
    float r, g, b, a;
} Vertex;

typedef struct {
    float time;
} UniformBuffer;

static Vertex vertices[] = {
    {0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},     // top vertex
    {-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},   // bottom left vertex
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},     // bottom right vertex
};

static void movement(const InputManager* inputManager, Player* player, float deltaTime) {
    const Vector2 forward = { cosf(player->angle), sinf(player->angle) };
    const Vector2 right = { -sinf(player->angle), cosf(player->angle) };

    float moveAmount = 100.0f * deltaTime; // adjust this

    if (input_manager_get_key(inputManager, SDL_SCANCODE_W)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(forward, moveAmount));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_S)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(forward, -moveAmount));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_D)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(right, moveAmount));
    }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_A)) {
        player->position = vector2_add(player->position, vector2_multiply_with_float(right, -moveAmount));
    }

    if (input_manager_get_key(inputManager, SDL_SCANCODE_RIGHT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_E)) {
        player->angle -= PLAYER_ROT_SPEED * deltaTime;
        }
    if (input_manager_get_key(inputManager, SDL_SCANCODE_LEFT) ||
        input_manager_get_key(inputManager, SDL_SCANCODE_Q)) {
        player->angle += PLAYER_ROT_SPEED * deltaTime;
        }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    SDL_Window *window = SDL_CreateWindow("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return -1;
    }

    SDL_GPUDevice* device = SDL_CreateGPUDevice(
    SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL,false, 0);
    if (!device) {
        SDL_Log("Couldn't create GPU device: %s", SDL_GetError());
        return -1;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Couldn't claim window: %s", SDL_GetError());
        return -1;
    }

    SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .size = sizeof(vertices),
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    };
    SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(device, &vertexBufferInfo);
    if (!vertexBuffer) {
        SDL_Log("Failed to create vertex buffer: %s", SDL_GetError());
        return -1;
    }
    SDL_GPUTransferBufferCreateInfo transferBufferInfo = {
        .size = sizeof(vertices),
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferBufferInfo);

    Vertex* data = (Vertex*)SDL_MapGPUTransferBuffer(device, transferBuffer, false);
    memcpy(data, vertices, sizeof(vertices));
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCommandBuffer* cBuffer = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cBuffer);
    SDL_GPUTransferBufferLocation location = {
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion region = {
        .buffer = vertexBuffer,
        .size = sizeof(vertices),
        .offset = 0,
    };
    SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cBuffer);

    size_t vertexShaderCodeSize;
    void* vertexShaderCode = SDL_LoadFile("cmake-build-debug/shaders/triangle.vertex.dxil", &vertexShaderCodeSize);
    if (!vertexShaderCode) {
        SDL_Log("Can't find vertex shader code %s", SDL_GetError());
        return -1;
    }
    SDL_GPUShaderCreateInfo vertexInfo = {
        .code = (Uint8*) vertexShaderCode,
        .code_size = vertexShaderCodeSize,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_DXIL,
        .stage = SDL_GPU_SHADERSTAGE_VERTEX,
        .num_samplers = 0,
        .num_storage_buffers = 0,
        .num_storage_textures = 0,
        .num_uniform_buffers = 0,
    };
    SDL_GPUShader* vertexShader = SDL_CreateGPUShader(device, &vertexInfo);
    if (!vertexShader) {
        SDL_Log("Couldn't create vertex shader: %s", SDL_GetError());
        return -1;
    }
    SDL_free(vertexShaderCode);

    size_t fragmentShaderCodeSize;
    void* fragmentShaderCode = SDL_LoadFile("cmake-build-debug/shaders/triangle.frag.dxil", &fragmentShaderCodeSize);
    if (!fragmentShaderCode) {
        SDL_Log("Can't fragment shader code: %s", SDL_GetError());
        return -1;
    }
    SDL_GPUShaderCreateInfo fragmentInfo = {
        .code = (Uint8*) fragmentShaderCode,
        .code_size = fragmentShaderCodeSize,
        .entrypoint = "main",
        .format = SDL_GPU_SHADERFORMAT_DXIL,
        .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .num_samplers = 0,
        .num_storage_buffers = 0,
        .num_storage_textures = 0,
        .num_uniform_buffers = 1,
    };
    SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(device, &fragmentInfo);
    if (!fragmentShader) {
        SDL_Log("Couldn't create vertex shader: %s", SDL_GetError());
        return -1;
    }
    SDL_free(fragmentShaderCode);

    SDL_GPUGraphicsPipelineCreateInfo pipeLineInfo = {
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
    };
    SDL_GPUVertexBufferDescription vertexBufferDescription[] = {
        {
            .slot = 0,
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
            .pitch = sizeof(Vertex),
        },
    };
    pipeLineInfo.vertex_input_state.num_vertex_buffers = sizeof(vertexBufferDescription) / sizeof(vertexBufferDescription[0]);
    pipeLineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescription;

    SDL_GPUVertexAttribute vertexAttributes[] = {
        {
            .buffer_slot = 0,
            .location = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0,
        },
        {
            .buffer_slot = 0,
            .location = 1,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
            .offset = 3 * sizeof(float),
        }
    };
    pipeLineInfo.vertex_input_state.num_vertex_attributes = sizeof(vertexAttributes) / sizeof(vertexAttributes[0]);
    pipeLineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

    SDL_GPUColorTargetDescription colorTargetDescriptions[] = {
        {
            .format = SDL_GetGPUSwapchainTextureFormat(device, window),
        }
    };
    pipeLineInfo.target_info.num_color_targets = sizeof(colorTargetDescriptions)/sizeof(colorTargetDescriptions[0]);
    pipeLineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

    SDL_GPUGraphicsPipeline* graphicsPipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeLineInfo);
    SDL_ReleaseGPUShader(device, vertexShader);
    SDL_ReleaseGPUShader(device, fragmentShader);

    UniformBuffer timeBuffer = {0};

    Player player = {.position = {0, 0}, .scale = 15.0f, .velocity = {0, 0}, .speed = .1f, .friction = 3.0f, .angle = 0};
    InputManager inputManager = {0};

    WallsList wallsList;
    physics_create_walls_list(&wallsList, 8);
    Wall walls[] = {
        {
            .position = {100.0f, 0.0f},
            .scale = {10.0f, 120.0f},
            .color = {255,255,255,255},
            .textures = {0,0,0,0},
            .height = 150.0f,
            .faceBrightness = {0,0,0,0}
        },
    };;
    for (int i = 0; i < sizeof(walls) / sizeof(walls[0]); i++) {
        physics_push_walls_list(&wallsList , &walls[i]);
    }

    bool running = true;
    Uint64 lastTime = SDL_GetTicksNS();
    while (running) {
        Uint64 currentTime = SDL_GetTicksNS();
        Uint32 startTime = SDL_GetTicks();
        float deltaTime = (float)(currentTime - lastTime) / 1000000000.0f;
        lastTime = currentTime;

        input_manager_begin_frame(&inputManager);

        if (input_manager_get_key_down(&inputManager, SDL_SCANCODE_ESCAPE)) running = false;
        movement(&inputManager, &player, deltaTime);
        player_update(&player);

        SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
        SDL_GPUTexture* swapChainTexture;
        Uint32 width, height;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapChainTexture, &width, &height)) {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            continue;
        }
        SDL_GPUColorTargetInfo colorTargetInfo = {
            .clear_color = {1.0f, .8f, .4f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .texture = swapChainTexture,
        };
        if (!swapChainTexture) {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
            continue;
        }
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderPass, graphicsPipeline);
        if (!graphicsPipeline) {
            SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
            return -1;
        }
        SDL_GPUBufferBinding bufferBindings[] = {
            {
                .buffer = vertexBuffer,
                .offset = 0,
            },
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, sizeof(bufferBindings)/sizeof(bufferBindings[0]));

        timeBuffer.time = SDL_GetTicksNS() / 1e9f;
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &timeBuffer, sizeof(UniformBuffer));
        SDL_DrawGPUPrimitives(renderPass, sizeof(vertices)/sizeof(vertices[0]), 1, 0, 0);

        SDL_EndGPURenderPass(renderPass);
        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }


    SDL_ReleaseGPUBuffer(device, vertexBuffer);
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    SDL_ReleaseGPUGraphicsPipeline(device, graphicsPipeline);

    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);

    physics_free_walls_list(&wallsList);
    SDL_Quit();
    return 0;
}
