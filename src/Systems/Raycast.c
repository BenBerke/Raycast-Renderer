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
    const float fovRadians = FOV * (M_PI / 180.0f);
    const float projectionPlane = (SCREEN_WIDTH / 2.0f) / tanf(fovRadians / 2.0f);

    for (int i = 0; i < RAY_COUNT; i++) {
        // 1. Calculate ray direction using atan2 logic to prevent fisheye
        float cameraX = (2.0f * ((float)i + 0.5f) / (float)RAY_COUNT) - 1.0f;
        float rayAngleOffset = atanf(cameraX * tanf(fovRadians * 0.5f));
        float rayAngle = player->angle - rayAngleOffset;
        Vector2 dir = { cosf(rayAngle), sinf(rayAngle) };

        float nearestT = FLT_MAX;
        int nearestSide = -1;
        const Wall* hitWall = NULL;

        // 2. Find ONLY the nearest wall
        for (int j = 0; j < walls->count; j++) {
            float t;
            int side;
            if (ray_intersect_wall(player->position, dir, &walls->items[j], &t, &side)) {
                if (t >= 0 && t < nearestT) {
                    nearestT = t;
                    nearestSide = side;
                    hitWall = &walls->items[j];
                }
            }
        }

        // 3. Fill the RaySlice for the GPU
        if (hitWall != NULL) {
            float correctedDist = nearestT * cosf(rayAngleOffset);
            if (correctedDist < 0.001f) correctedDist = 0.001f;

            outBuffer[i].wallHeight = (WALL_HEIGHT / correctedDist) * projectionPlane;

            // Calculate U (horizontal texture mapping)
            float hitX = player->position.x + dir.x * nearestT;
            float hitY = player->position.y + dir.y * nearestT;
            float u = 0.0f;
            if (nearestSide == 0 || nearestSide == 2) u = (hitX - (hitWall->position.x - hitWall->scale.x/2.0f)) / hitWall->scale.x;
            else u = (hitY - (hitWall->position.y - hitWall->scale.y/2.0f)) / hitWall->scale.y;

            outBuffer[i].u = fmaxf(0.0f, fminf(1.0f, u));
            outBuffer[i].textureIndex = hitWall->textures[nearestSide];

            // Distance-based shading
            float ambient = 0.2f;
            outBuffer[i].brightness = ambient + (1.0f - ambient) * (1.0f - fminf(correctedDist / 1000.0f, 1.0f));
        } else {
            outBuffer[i].wallHeight = 0.0f;
            outBuffer[i].u = 0.0f;
            outBuffer[i].textureIndex = 0;
            outBuffer[i].brightness = 0.0f;
        }
    }
}