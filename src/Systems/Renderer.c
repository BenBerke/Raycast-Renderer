#include "../../Headers/Systems/Renderer.h"

#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include "../../config.h"
#include "../../Headers/Systems/Raycast.h"

/* ------------------------------------------------------------------ */
/* Internal vertex data                                                 */
/* ------------------------------------------------------------------ */

static Vertex vertices[] = {
    { -1.0f,  1.0f, 0.0f, 0.0f, 0.0f }, // top left
    {  1.0f,  1.0f, 0.0f, 1.0f, 0.0f }, // top right
    { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f }, // bottom left
    {  1.0f, -1.0f, 0.0f, 1.0f, 1.0f }  // bottom right
};

/* ------------------------------------------------------------------ */
/* renderer_init                                                        */
/* ------------------------------------------------------------------ */

bool renderer_init(Renderer* renderer, SDL_Window* window) {
    /* GPU device */
    renderer->device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL, false, 0);
    if (!renderer->device) {
        SDL_Log("Couldn't create GPU device: %s", SDL_GetError());
        return false;
    }
    if (!SDL_ClaimWindowForGPUDevice(renderer->device, window)) {
        SDL_Log("Couldn't claim window: %s", SDL_GetError());
        return false;
    }

    /* Ray storage buffer */
    SDL_GPUBufferCreateInfo rayBufferInfo = {
        .size  = sizeof(RaySlice) * RAY_COUNT,
        .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
    };
    renderer->rayBuffer = SDL_CreateGPUBuffer(renderer->device, &rayBufferInfo);
    if (!renderer->rayBuffer) {
        SDL_Log("Failed to create ray buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTransferBufferCreateInfo rayTransferBufferInfo = {
        .size  = sizeof(RaySlice) * RAY_COUNT,
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    };
    renderer->rayTransferBuffer = SDL_CreateGPUTransferBuffer(renderer->device, &rayTransferBufferInfo);
    if (!renderer->rayTransferBuffer) {
        SDL_Log("Failed to create ray transfer buffer: %s", SDL_GetError());
        return false;
    }

    /* Vertex buffer */
    SDL_GPUBufferCreateInfo vertexBufferInfo = {
        .size  = sizeof(vertices),
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    };
    renderer->vertexBuffer = SDL_CreateGPUBuffer(renderer->device, &vertexBufferInfo);
    if (!renderer->vertexBuffer) {
        SDL_Log("Failed to create vertex buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUTransferBufferCreateInfo vertexTransferBufferInfo = {
        .size  = sizeof(vertices),
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    };
    renderer->vertexTransferBuffer = SDL_CreateGPUTransferBuffer(renderer->device, &vertexTransferBufferInfo);
    if (!renderer->vertexTransferBuffer) {
        SDL_Log("Failed to create vertex transfer buffer: %s", SDL_GetError());
        return false;
    }

    /* Upload vertex data once */
    Vertex* vertexData = (Vertex*)SDL_MapGPUTransferBuffer(renderer->device, renderer->vertexTransferBuffer, false);
    memcpy(vertexData, vertices, sizeof(vertices));
    SDL_UnmapGPUTransferBuffer(renderer->device, renderer->vertexTransferBuffer);

    SDL_GPUCommandBuffer* cBuffer = SDL_AcquireGPUCommandBuffer(renderer->device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cBuffer);
    SDL_GPUTransferBufferLocation location = {
        .transfer_buffer = renderer->vertexTransferBuffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion region = {
        .buffer = renderer->vertexBuffer,
        .size   = sizeof(vertices),
        .offset = 0,
    };
    SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cBuffer);

    /* Vertex shader */
    size_t vertexShaderCodeSize;
    void* vertexShaderCode = SDL_LoadFile("cmake-build-debug/shaders/triangle.vertex.dxil", &vertexShaderCodeSize);
    if (!vertexShaderCode) {
        SDL_Log("Can't find vertex shader code %s", SDL_GetError());
        return false;
    }
    SDL_GPUShaderCreateInfo vertexInfo = {
        .code                = (Uint8*)vertexShaderCode,
        .code_size           = vertexShaderCodeSize,
        .entrypoint          = "main",
        .format              = SDL_GPU_SHADERFORMAT_DXIL,
        .stage               = SDL_GPU_SHADERSTAGE_VERTEX,
        .num_samplers        = 0,
        .num_storage_buffers = 0,
        .num_storage_textures= 0,
        .num_uniform_buffers = 0,
    };
    SDL_GPUShader* vertexShader = SDL_CreateGPUShader(renderer->device, &vertexInfo);
    if (!vertexShader) {
        SDL_Log("Couldn't create vertex shader: %s", SDL_GetError());
        return false;
    }
    SDL_free(vertexShaderCode);

    /* Fragment shader */
    size_t fragmentShaderCodeSize;
    void* fragmentShaderCode = SDL_LoadFile("cmake-build-debug/shaders/triangle.frag.dxil", &fragmentShaderCodeSize);
    if (!fragmentShaderCode) {
        SDL_Log("Can't fragment shader code: %s", SDL_GetError());
        return false;
    }
    SDL_GPUShaderCreateInfo fragmentInfo = {
        .code                = (Uint8*)fragmentShaderCode,
        .code_size           = fragmentShaderCodeSize,
        .entrypoint          = "main",
        .format              = SDL_GPU_SHADERFORMAT_DXIL,
        .stage               = SDL_GPU_SHADERSTAGE_FRAGMENT,
        .num_samplers        = 0,
        .num_storage_buffers = 1,
        .num_storage_textures= 0,
        .num_uniform_buffers = 0,
    };
    SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(renderer->device, &fragmentInfo);
    if (!fragmentShader) {
        SDL_Log("Couldn't create vertex shader: %s", SDL_GetError());
        return false;
    }
    SDL_free(fragmentShaderCode);

    /* Graphics pipeline */
    SDL_GPUGraphicsPipelineCreateInfo pipeLineInfo = {
        .vertex_shader   = vertexShader,
        .fragment_shader = fragmentShader,
        .primitive_type  = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP,
    };

    SDL_GPUVertexBufferDescription vertexBufferDescription[] = {
        {
            .slot               = 0,
            .input_rate         = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
            .pitch              = sizeof(Vertex),
        },
    };
    pipeLineInfo.vertex_input_state.num_vertex_buffers        = sizeof(vertexBufferDescription) / sizeof(vertexBufferDescription[0]);
    pipeLineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDescription;

    SDL_GPUVertexAttribute vertexAttributes[] = {
        {
            .buffer_slot = 0,
            .location    = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = 0,
        },
        {
            .buffer_slot = 0,
            .location    = 1,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset      = 3 * sizeof(float),
        }
    };
    pipeLineInfo.vertex_input_state.num_vertex_attributes = sizeof(vertexAttributes) / sizeof(vertexAttributes[0]);
    pipeLineInfo.vertex_input_state.vertex_attributes     = vertexAttributes;

    SDL_GPUColorTargetDescription colorTargetDescriptions[] = {
        {
            .format = SDL_GetGPUSwapchainTextureFormat(renderer->device, window),
        }
    };
    pipeLineInfo.target_info.num_color_targets        = sizeof(colorTargetDescriptions) / sizeof(colorTargetDescriptions[0]);
    pipeLineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

    renderer->graphicsPipeline = SDL_CreateGPUGraphicsPipeline(renderer->device, &pipeLineInfo);
    if (!renderer->graphicsPipeline) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(renderer->device, vertexShader);
    SDL_ReleaseGPUShader(renderer->device, fragmentShader);

    return true;
}

/* ------------------------------------------------------------------ */
/* renderer_render_frame                                                */
/* ------------------------------------------------------------------ */

bool renderer_render_frame(Renderer* renderer, SDL_Window* window, RaySlice* cpuSlices) {
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(renderer->device);

    SDL_GPUTexture* swapChainTexture;
    Uint32 width, height;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapChainTexture, &width, &height)) {
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return false;
    }
    if (!swapChainTexture) {
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return false;
    }

    /* Upload ray slices */
    RaySlice* mapped = (RaySlice*)SDL_MapGPUTransferBuffer(renderer->device, renderer->rayTransferBuffer, true);
    memcpy(mapped, cpuSlices, sizeof(RaySlice) * RAY_COUNT);
    SDL_UnmapGPUTransferBuffer(renderer->device, renderer->rayTransferBuffer);

    SDL_GPUCopyPass* rayCopyPass = SDL_BeginGPUCopyPass(commandBuffer);
    SDL_GPUTransferBufferLocation raySrc = {
        .transfer_buffer = renderer->rayTransferBuffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion rayDst = {
        .buffer = renderer->rayBuffer,
        .offset = 0,
        .size   = sizeof(RaySlice) * RAY_COUNT,
    };
    SDL_UploadToGPUBuffer(rayCopyPass, &raySrc, &rayDst, true);
    SDL_EndGPUCopyPass(rayCopyPass);

    /* Render pass */
    SDL_GPUColorTargetInfo colorTargetInfo = {
        .clear_color = {1.0f, .8f, .4f, 1.0f},
        .load_op     = SDL_GPU_LOADOP_CLEAR,
        .store_op    = SDL_GPU_STOREOP_STORE,
        .texture     = swapChainTexture,
    };

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

    SDL_GPUBuffer* fragmentStorageBuffers[] = { renderer->rayBuffer };
    SDL_BindGPUFragmentStorageBuffers(renderPass, 0, fragmentStorageBuffers, 1);

    SDL_BindGPUGraphicsPipeline(renderPass, renderer->graphicsPipeline);

    SDL_GPUBufferBinding bufferBindings[] = {
        {
            .buffer = renderer->vertexBuffer,
            .offset = 0,
        },
    };
    SDL_BindGPUVertexBuffers(renderPass, 0, bufferBindings, sizeof(bufferBindings) / sizeof(bufferBindings[0]));

    SDL_DrawGPUPrimitives(renderPass, sizeof(vertices) / sizeof(vertices[0]), 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return true;
}

/* ------------------------------------------------------------------ */
/* renderer_destroy                                                     */
/* ------------------------------------------------------------------ */

void renderer_destroy(Renderer* renderer) {
    SDL_ReleaseGPUBuffer(renderer->device, renderer->vertexBuffer);
    SDL_ReleaseGPUTransferBuffer(renderer->device, renderer->vertexTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(renderer->device, renderer->rayTransferBuffer);
    SDL_ReleaseGPUGraphicsPipeline(renderer->device, renderer->graphicsPipeline);
    SDL_DestroyGPUDevice(renderer->device);
}