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
    const float fovRadians = FOV * (float)M_PI / 180.0f;
    const float halfFov = fovRadians * 0.5f;
    const float projectionPlane = (SCREEN_WIDTH * 0.5f) / tanf(halfFov);

    for (int i = 0; i < RAY_COUNT; i++) {
        float cameraX = (2.0f * ((float)i + 0.5f) / (float)RAY_COUNT) - 1.0f;
        float rayAngleOffset = atanf(cameraX * tanf(halfFov));
        float rayAngle = player->angle + rayAngleOffset;

        Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

        float nearestT = FLT_MAX;
        int nearestSide = -1;

        for (int j = 0; j < walls->count; j++) {
            float t;
            int side;

            if (ray_intersect_wall(player->position, dir, &walls->items[j], &t, &side)) {
                if (t > 0.0f && t < nearestT) {
                    nearestT = t;
                    nearestSide = side;
                }
            }
        }

        if (nearestSide == -1) {
            outBuffer[i].top = .5f;
            outBuffer[i].bottom = 0.5f;
            outBuffer[i].brightness = 0.0f;
            outBuffer[i].distance = -1.0f;
            continue;
        }

        float correctedDistance = nearestT * cosf(rayAngleOffset);
        if (correctedDistance < 0.0001f) {
            correctedDistance = 0.0001f;
        }

        float wallHeightPixels = (WALL_HEIGHT * projectionPlane) / correctedDistance;
        float wallHeightUV = wallHeightPixels / (float)SCREEN_HEIGHT;

        float top = 0.5f - wallHeightUV * 0.5f;
        float bottom = 0.5f + wallHeightUV * 0.5f;

        if (top < 0.0f) top = 0.0f;
        if (bottom > 1.0f) bottom = 1.0f;

        float sideShade = (nearestSide == 1 || nearestSide == 3) ? 0.85f : 1.0f;
        float distanceFade = 1.0f / (1.0f + correctedDistance * 0.02f);
        float brightness = sideShade * distanceFade;
        if (brightness < 0.15f) brightness = 0.15f;

        outBuffer[i].top = top;
        outBuffer[i].bottom = bottom;
        outBuffer[i].brightness = brightness;
        outBuffer[i].distance = correctedDistance;
    }
}