#define USE_MATH_DEFINES
#include "../../Headers/Systems/Raycast.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../config.h"

// Keeps the core AABB ray-box intersection math
bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT, int* outSide) {
    const float minX = wall->position.x - wall->scale.x / 2.0f;
    const float maxX = wall->position.x + wall->scale.x / 2.0f;
    const float minY = wall->position.y - wall->scale.y / 2.0f;
    const float maxY = wall->position.y + wall->scale.y / 2.0f;

    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;
    int enterSide = -1;

    // X-axis check
    if (fabsf(dir.x) > 0.00001f) {
        float tx1 = (minX - origin.x) / dir.x;
        float tx2 = (maxX - origin.x) / dir.x;
        if (tx1 > tx2) { float tmp = tx1; tx1 = tx2; tx2 = tmp; }
        if (tx1 > tMin) { tMin = tx1; enterSide = (dir.x > 0) ? 3 : 1; }
        if (tx2 < tMax) tMax = tx2;
    } else if (origin.x < minX || origin.x > maxX) return false;

    // Y-axis check
    if (fabsf(dir.y) > 0.00001f) {
        float ty1 = (minY - origin.y) / dir.y;
        float ty2 = (maxY - origin.y) / dir.y;
        if (ty1 > ty2) { float tmp = ty1; ty1 = ty2; ty2 = tmp; }
        if (ty1 > tMin) { tMin = ty1; enterSide = (dir.y > 0) ? 0 : 2; }
        if (ty2 < tMax) tMax = ty2;
    } else if (origin.y < minY || origin.y > maxY) return false;

    if (tMin > tMax || tMax < 0.0f) return false;

    *outT = tMin;
    *outSide = enterSide;
    return true;
}

// The "Collector" function: Finds the nearest wall and prepares the GPU data
void raycast_to_gpu_buffer(const Player* player, const WallsList* walls, RaySlice* outBuffer) {
}