#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include <stdbool.h>

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "Physics.h"

typedef struct {
<<<<<<< HEAD
} RaySlice;
=======
    float distance;
    unsigned char r, g, b, a;
    char side;      // 0 top, 1 bottom, 2 left, 3 right
    float u;        // 0.0 -> 1.0 across the wall face
    int textures[4];
    float height;
    float faceBrightness[4];
} RayReturn;
>>>>>>> parent of ca60d0c (Dekat Time)


void raycast_to_gpu_buffer(const Player* player, const WallsList* walls, RaySlice* outBuffer);
bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT, int* outSide);

#endif