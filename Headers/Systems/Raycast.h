#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include <stdbool.h>

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "../AppState.h"
#include "Physics.h"

typedef struct {
    float wallHeight;   // Derived from distance: (1.0 / dist) * projectionPlane
    float u;            // The horizontal hit point (0.0 to 1.0)
    int textureIndex;   // Which texture to sample
    float brightness;   // We'll use this for the "distance fade" or side-shading
} RaySlice;


void raycast_to_gpu_buffer(const Player* player, const WallsList* walls, RaySlice* outBuffer);
bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT, int* outSide);
bool renderer_create_raySlice_data_buffer(AppState* state, RaySlice* raySlices, int sliceCount);

#endif