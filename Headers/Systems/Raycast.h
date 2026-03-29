#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include <stdbool.h>

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "Physics.h"

typedef struct {
    float top;
    float bottom;
    float brightness;
    float distance;
} RaySlice;


void raycast_to_gpu_buffer(const Player* player, const WallsList* walls, RaySlice* outBuffer);
bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT, int* outSide);

#endif