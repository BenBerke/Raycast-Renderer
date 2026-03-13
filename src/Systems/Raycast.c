#include "../../Headers/Systems/Raycast.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "../../config.h"

static bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT, int* outSide) {
    const float minX = wall->position.x - wall->scale.x / 2.0f;
    const float maxX = wall->position.x + wall->scale.x / 2.0f;
    const float minY = wall->position.y - wall->scale.y / 2.0f;
    const float maxY = wall->position.y + wall->scale.y / 2.0f;

    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;

    int enterSide = -1;
    int exitSide = -1;

    if (fabsf(dir.x) < 0.00001f) {
        if (origin.x < minX || origin.x > maxX) {
            return false;
        }
    } else {
        float tx1 = (minX - origin.x) / dir.x;
        float tx2 = (maxX - origin.x) / dir.x;
        int tx1Side = 2;
        int tx2Side = 3;

        if (tx1 > tx2) {
            float tempT = tx1;
            tx1 = tx2;
            tx2 = tempT;

            int tempSide = tx1Side;
            tx1Side = tx2Side;
            tx2Side = tempSide;
        }

        if (tx1 > tMin) {
            tMin = tx1;
            enterSide = tx1Side;
        }
        if (tx2 < tMax) {
            tMax = tx2;
            exitSide = tx2Side;
        }
    }

    if (fabsf(dir.y) < 0.00001f) {
        if (origin.y < minY || origin.y > maxY) {
            return false;
        }
    } else {
        float ty1 = (minY - origin.y) / dir.y;
        float ty2 = (maxY - origin.y) / dir.y;
        int ty1Side = 1;
        int ty2Side = 0;

        if (ty1 > ty2) {
            float tempT = ty1;
            ty1 = ty2;
            ty2 = tempT;

            int tempSide = ty1Side;
            ty1Side = ty2Side;
            ty2Side = tempSide;
        }

        if (ty1 > tMin) {
            tMin = ty1;
            enterSide = ty1Side;
        }
        if (ty2 < tMax) {
            tMax = ty2;
            exitSide = ty2Side;
        }
    }

    if (tMin > tMax) return false;
    if (tMax < 0.0f) return false;

    if (tMin >= 0.0f) {
        *outT = tMin;
        *outSide = enterSide;
    } else {
        *outT = tMax;
        *outSide = exitSide;
    }

    return true;
}

RayReturn raycast_create_ray(Ray* ray, const Player* player, Vector2 dir, const WallsList* list) {
    RayReturn result = {-1.0f, 0, 0, 0, -1, 0.0f};

    const Vector2 origin = player->position;
    float closestT = (float)MAX_RAY_LENGTH;
    const Wall* closestWall = NULL;
    int closestSide = -1;

    for (int i = 0; i < list->count; i++) {
        float t = 0.0f;
        int side = -1;

        if (ray_intersect_wall(origin, dir, &list->items[i], &t, &side)) {
            if (t >= 0.0f && t < closestT) {
                closestT = t;
                closestWall = &list->items[i];
                closestSide = side;
            }
        }
    }

    if (closestWall == NULL) {
        ray->position.x = origin.x + dir.x * MAX_RAY_LENGTH;
        ray->position.y = origin.y + dir.y * MAX_RAY_LENGTH;
        return result;
    }

    ray->position.x = origin.x + dir.x * closestT;
    ray->position.y = origin.y + dir.y * closestT;

    result.distance = closestT;
    result.r = (unsigned char)closestWall->color.x;
    result.g = (unsigned char)closestWall->color.y;
    result.b = (unsigned char)closestWall->color.z;
    result.side = (char)closestSide;

    const float minX = closestWall->position.x - closestWall->scale.x / 2.0f;
    const float minY = closestWall->position.y - closestWall->scale.y / 2.0f;

    float u = 0.0f;
    switch (result.side) {
        case 0:
        case 1:
            u = (ray->position.x - minX) / closestWall->scale.x;
            break;

        case 2:
        case 3:
            u = (ray->position.y - minY) / closestWall->scale.y;
            break;

        default:
            u = 0.0f;
            break;
    }

    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;

    result.u = u;
    return result;
}