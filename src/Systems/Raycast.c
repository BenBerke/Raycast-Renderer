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
<<<<<<< HEAD
        if (tx1 > tx2) { float tmp = tx1; tx1 = tx2; tx2 = tmp; }
        if (tx1 > tMin) { tMin = tx1; enterSide = (dir.x > 0) ? 3 : 1; }
        if (tx2 < tMax) tMax = tx2;
    } else if (origin.x < minX || origin.x > maxX) return false;
=======
        int tx1Side = 2;
        int tx2Side = 3;
>>>>>>> parent of ca60d0c (Dekat Time)

    // Y-axis check
    if (fabsf(dir.y) > 0.00001f) {
        float ty1 = (minY - origin.y) / dir.y;
        float ty2 = (maxY - origin.y) / dir.y;
<<<<<<< HEAD
        if (ty1 > ty2) { float tmp = ty1; ty1 = ty2; ty2 = tmp; }
        if (ty1 > tMin) { tMin = ty1; enterSide = (dir.y > 0) ? 0 : 2; }
        if (ty2 < tMax) tMax = ty2;
    } else if (origin.y < minY || origin.y > maxY) return false;
=======
        int ty1Side = 1;
        int ty2Side = 0;
>>>>>>> parent of ca60d0c (Dekat Time)

    if (tMin > tMax || tMax < 0.0f) return false;

    *outT = tMin;
    *outSide = enterSide;
    return true;
}

<<<<<<< HEAD
// The "Collector" function: Finds the nearest wall and prepares the GPU data
void raycast_to_gpu_buffer(const Player* player, const WallsList* walls, RaySlice* outBuffer) {
=======
static void sort_hits_far_to_near(RayReturn* hits, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (hits[i].distance < hits[j].distance) {
                RayReturn temp = hits[i];
                hits[i] = hits[j];
                hits[j] = temp;
            }
        }
    }
}

int raycast_collect_hits(
    Ray* nearestRay,
    const Player* player,
    const Vector2 dir,
    const WallsList* list,
    RayReturn* outHits,
    const int maxHits
) {
    const Vector2 origin = player->position;

    int hitCount = 0;
    float nearestT = (float)MAX_RAY_LENGTH;
    bool foundNearest = false;

    for (int i = 0; i < maxHits; i++) {
        outHits[i] = (RayReturn){-1.0f, 0, 0, 0, -1, 0, {0, 0, 0, 0}, 0};
    }

    for (int i = 0; i < list->count; i++) {
        float t = 0.0f;
        int side = -1;
        const Wall* wall = &list->items[i];

        if (!ray_intersect_wall(origin, dir, wall, &t, &side)) {
            continue;
        }
        if (t < 0.0f) {
            continue;
        }

        if (t < nearestT) {
            nearestT = t;
            foundNearest = true;
        }

        if (hitCount >= maxHits) {
            continue;
        }

        const float hitX = origin.x + dir.x * t;
        const float hitY = origin.y + dir.y * t;

        const float minX = wall->position.x - wall->scale.x / 2.0f;
        const float minY = wall->position.y - wall->scale.y / 2.0f;

        float u = 0.0f;
        switch (side) {
            case 0:
            case 1:
                u = (hitX - minX) / wall->scale.x;
                break;

            case 2:
            case 3:
                u = (hitY - minY) / wall->scale.y;
                break;

            default:
                u = 0.0f;
                break;
        }

        if (u < 0.0f) u = 0.0f;
        if (u > 1.0f) u = 1.0f;

        outHits[hitCount] = (RayReturn){
            .distance = t,
            .r = (unsigned char)wall->color.x,
            .g = (unsigned char)wall->color.y,
            .b = (unsigned char)wall->color.z,
            .a = (unsigned char)wall->color.q,
            .side = (char)side,
            .u = u,
            .textures = -1,
            .height = wall->height,
            .faceBrightness = 0,
        };
        memcpy(outHits[hitCount].textures, wall->textures, 4 * sizeof(int));
        memcpy(outHits[hitCount].faceBrightness, wall->faceBrightness, 4 * sizeof(float));

        hitCount++;
    }

    if (foundNearest) {
        nearestRay->position.x = origin.x + dir.x * nearestT;
        nearestRay->position.y = origin.y + dir.y * nearestT;
    } else {
        nearestRay->position.x = origin.x + dir.x * MAX_RAY_LENGTH;
        nearestRay->position.y = origin.y + dir.y * MAX_RAY_LENGTH;
    }

    sort_hits_far_to_near(outHits, hitCount);
    return hitCount;
>>>>>>> parent of ca60d0c (Dekat Time)
}