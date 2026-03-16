#include "../../Headers/Systems/Renderer.h"

#include <stdio.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>

#include "../../Headers/Systems/Raycast.h"

SDL_GPUShader* renderer_load_shader(SDL_GPUDevice* device, const char* fileName) {
    if (device == NULL || fileName == NULL) {
        SDL_Log("renderer_load_shader: device or fileName was NULL");
        return NULL;
    }

    SDL_GPUShaderStage stage;
    if (strstr(fileName, ".vert") != NULL) {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    } else if (strstr(fileName, ".frag") != NULL) {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    } else {
        SDL_Log("renderer_load_shader: shader file must contain .vert or .frag: %s", fileName);
        return NULL;
    }

    const char* basePath = SDL_GetBasePath();
    if (basePath == NULL) {
        SDL_Log("renderer_load_shader: SDL_GetBasePath failed: %s", SDL_GetError());
        return NULL;
    }

    char fullPath[512];
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    const char* entryPoint = "main";

    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);

    /* Prefer DXIL first if available, since you're debugging D3D12 */
    if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.dxil", basePath, fileName);
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entryPoint = "main";
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.spv", basePath, fileName);
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entryPoint = "main";
    } else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
        SDL_snprintf(fullPath, sizeof(fullPath), "%sshaders/%s.msl", basePath, fileName);
        format = SDL_GPU_SHADERFORMAT_MSL;
        entryPoint = "main0";
    } else {
        SDL_Log("renderer_load_shader: no compatible shader format for driver %s",
                SDL_GetGPUDeviceDriver(device));
        return NULL;
    }

    size_t fileSize = 0;
    Uint8* code = (Uint8*) SDL_LoadFile(fullPath, &fileSize);
    if (code == NULL) {
        SDL_Log("renderer_load_shader: couldn't load shader file %s: %s", fullPath, SDL_GetError());
        return NULL;
    }

    SDL_GPUShaderCreateInfo shaderInfo = {
        .code_size = fileSize,
        .code = code,
        .entrypoint = entryPoint,
        .format = format,
        .stage = stage,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? 1 : 0,
        .num_uniform_buffers = 0,
        .props = 0
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (shader == NULL) {
        SDL_Log("renderer_load_shader: couldn't create shader from %s: %s", fullPath, SDL_GetError());
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);
    return shader;
}

bool renderer_update_raySlice_data_buffer(AppState* state, const RaySlice* rayData, Uint32 count) {
    if (state == NULL || state->device == NULL || state->rayStorageBuffer == NULL || rayData == NULL) {
        SDL_Log("renderer_update_raySlice_data_buffer: invalid arguments");
        return false;
    }

    const Uint32 size = (Uint32)(sizeof(RaySlice) * count);

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(
        state->device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = size,
            .props = 0
        }
    );

    if (transferBuffer == NULL) {
        SDL_Log("Failed to create ray slice transfer buffer: %s", SDL_GetError());
        return false;
    }

    void* mapped = SDL_MapGPUTransferBuffer(state->device, transferBuffer, false);
    if (mapped == NULL) {
        SDL_Log("Failed to map ray slice transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
        return false;
    }

    SDL_memcpy(mapped, rayData, size);
    SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(state->device);
    if (commandBuffer == NULL) {
        SDL_Log("Failed to acquire GPU command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
        return false;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
    if (copyPass == NULL) {
        SDL_Log("Failed to begin GPU copy pass: %s", SDL_GetError());
        SDL_CancelGPUCommandBuffer(commandBuffer);
        SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
        return false;
    }

    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transferBuffer,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = state->rayStorageBuffer,
            .offset = 0,
            .size = size
        },
        false
    );

    SDL_EndGPUCopyPass(copyPass);

    if (!SDL_SubmitGPUCommandBuffer(commandBuffer)) {
        SDL_Log("Failed to submit GPU command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
        return false;
    }

    SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
    return true;
}

bool renderer_create_pipeline(AppState* state) {
    SDL_GPUShader* vertexShader = renderer_load_shader(state->device, "Raycast.vert");
    if (vertexShader == NULL) {
        SDL_Log("Unable to load vertex shader: %s", SDL_GetError());
        return false;
    }

    SDL_GPUShader* fragmentShader = renderer_load_shader(state->device, "SolidColor.frag");
    if (fragmentShader == NULL) {
        SDL_Log("Unable to load fragment shader: %s", SDL_GetError());
        SDL_ReleaseGPUShader(state->device, vertexShader);
        return false;
    }

    SDL_GPUTextureFormat swapchainFormat =
        SDL_GetGPUSwapchainTextureFormat(state->device, state->window);

    SDL_GPUColorTargetDescription colorTargetDescriptions[] = {
        {
            .format = swapchainFormat
        },
    };

    SDL_GPUVertexBufferDescription vertexBufferDesc = {
        .slot = 0,
        .pitch = sizeof(Vertex),
        .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
        .instance_step_rate = 0
    };

    SDL_GPUVertexAttribute vertexAttrib = {
        .location = 0,
        .buffer_slot = 0,
        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
        .offset = 0
    };

    SDL_GPUVertexInputState vertexInputState = {
        .vertex_buffer_descriptions = &vertexBufferDesc,
        .num_vertex_buffers = 1,
        .vertex_attributes = &vertexAttrib,
        .num_vertex_attributes = 1
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .vertex_shader = vertexShader,
        .fragment_shader = fragmentShader,
        .vertex_input_state = vertexInputState,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,

        .rasterizer_state = {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_NONE,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },

        .multisample_state = {
            .sample_count = SDL_GPU_SAMPLECOUNT_1,
            .sample_mask = 0,
            .enable_mask = false,
        },

        .depth_stencil_state = {0},

        .target_info = {
            .color_target_descriptions = colorTargetDescriptions,
            .num_color_targets = 1,
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_INVALID,
            .has_depth_stencil_target = false
        },

        .props = 0
    };

    state->pipeline = SDL_CreateGPUGraphicsPipeline(state->device, &pipelineCreateInfo);
    if (state->pipeline == NULL) {
        SDL_Log("Couldn't create graphics pipeline: %s", SDL_GetError());
        SDL_ReleaseGPUShader(state->device, vertexShader);
        SDL_ReleaseGPUShader(state->device, fragmentShader);
        return false;
    }

    SDL_ReleaseGPUShader(state->device, vertexShader);
    SDL_ReleaseGPUShader(state->device, fragmentShader);
    return true;
}

bool renderer_create_raySlice_data_buffer(AppState* state, RaySlice* raySlices, int sliceCount) {
    state->numSlices = sliceCount;
    Uint32 slicesSize = sliceCount * sizeof(RaySlice);

    SDL_GPUBufferCreateInfo dataBufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
        .size = slicesSize,
    };
    state->rayStorageBuffer = SDL_CreateGPUBuffer(state->device, &dataBufferCreateInfo);
    if (state->rayStorageBuffer == 0) {
        SDL_Log("Couldn't create ray data buffer %s", SDL_GetError());
        return false;
    }
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = slicesSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(state->device, &transferBufferCreateInfo);
    if (transferBuffer == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    Uint16* transferData = (void*)SDL_MapGPUTransferBuffer(state->device, transferBuffer, false);
    if (transferData == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    memcpy(transferData, raySlices, slicesSize);
    SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(state->device);
    if (uploadCmdBuf == NULL) {
        SDL_Log("Couldn't create command buffer %s", SDL_GetError());
        return false;
    }
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_GPUTransferBufferLocation bufferLocation = {
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion bufferRegion = {
        .buffer = state->rayStorageBuffer,
        .offset = 0,
        .size = slicesSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
        SDL_Log("Couldn't submit command buffer %s", SDL_GetError());
        return false;
    }
    SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
    return true;
}

bool renderer_create_index_buffer(AppState* state, Uint16* indices, int indexCount) {
    state->numIndices = indexCount;
    Uint32 indicesSize = indexCount * sizeof(Uint16);

    SDL_GPUBufferCreateInfo vertexBufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = indicesSize,
    };
    state->indexBuffer = SDL_CreateGPUBuffer(state->device, &vertexBufferCreateInfo);
    if (state->indexBuffer == 0) {
        SDL_Log("Couldn't create index buffer %s", SDL_GetError());
        return false;
    }
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = indicesSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(state->device, &transferBufferCreateInfo);
    if (transferBuffer == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    Uint16* transferData = (Uint16*)SDL_MapGPUTransferBuffer(state->device, transferBuffer, false);
    if (transferData == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    memcpy(transferData, indices, indicesSize);
    SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(state->device);
    if (uploadCmdBuf == NULL) {
        SDL_Log("Couldn't create command buffer %s", SDL_GetError());
        return false;
    }
    SDL_GPUCopyPass* copyPass =  SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_GPUTransferBufferLocation bufferLocation = {
        .transfer_buffer = transferBuffer,
        .offset =  0,
    };
    SDL_GPUBufferRegion bufferRegion = {
        .buffer = state->indexBuffer,
        .offset = 0,
        .size = indicesSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
        SDL_Log("Couldn't submit command buffer %s", SDL_GetError());
        return false;
    }
    SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
    return true;
}

bool renderer_create_vertex_buffer(AppState* state, Vertex* vertices, int vertexCount) {
    state->numVertices = vertexCount;
    Uint32 verticesSize = state->numVertices * sizeof(Vertex);
    SDL_GPUBufferCreateInfo vertexBufferCreateInfo = {
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = verticesSize,
    };
    state->vertexBuffer = SDL_CreateGPUBuffer(state->device, &vertexBufferCreateInfo);
    if (state->vertexBuffer == 0) {
        SDL_Log("Couldn't create vertex buffer %s", SDL_GetError());
        return false;
    }
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = verticesSize,
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(state->device, &transferBufferCreateInfo);
    if (transferBuffer == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        return false;
    }
    Vertex* transferData = (Vertex*)SDL_MapGPUTransferBuffer(state->device, transferBuffer, false);
    if (transferData == NULL) {
        SDL_Log("Couldn't create transfer buffer %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
        return false;
    }
    memcpy(transferData, vertices, verticesSize);
    SDL_UnmapGPUTransferBuffer(state->device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(state->device);
    if (uploadCmdBuf == NULL) {
        SDL_Log("Couldn't create command buffer %s", SDL_GetError());
        return false;
    }
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    SDL_GPUTransferBufferLocation bufferLocation = {
        .transfer_buffer = transferBuffer,
        .offset = 0,
    };
    SDL_GPUBufferRegion bufferRegion = {
        .buffer = state->vertexBuffer,
        .offset = 0,
        .size = verticesSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, false);
    SDL_EndGPUCopyPass(copyPass);
    if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf)) {
        SDL_Log("Couldn't submit command buffer %s", SDL_GetError());
        return false;
    }
    SDL_ReleaseGPUTransferBuffer(state->device, transferBuffer);
    return true;
}