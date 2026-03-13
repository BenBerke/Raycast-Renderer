//
// Created by berke on 3/12/2026.
//

#include <math.h>

#include "../../Headers/Systems/Raycast.h"

#include <stdlib.h>

#include "../../config.h"
#include "../../Headers/Systems/Physics.h"

RayReturn raycast_create_ray(Ray* ray, Player* p, const Vector2 dir, const WallsList* list) {
    RayReturn result = {-1.0f, 0, 0, 0, 0, 0};

    const Vector2 origin = p->position;

    float closestT = (float)MAX_RAY_LENGTH;
    const Wall* closestWall = NULL;

    int closestSide = -1;
    for (int i = 0; i < list->count; i++) {
        float t;
        int side;
        if (ray_intersect_wall(origin, dir, &list->items[i], &t, &side)) {
            if (t >= 0.0f && t < closestT) {
                closestT = t;
                closestWall = &list->items[i];
                closestSide = side;
                result.side = closestSide;
            }
        }
    }

    if (closestWall) {
        ray->position.x = origin.x + dir.x * closestT;
        ray->position.y = origin.y + dir.y * closestT;

        result.distance = closestT;
        result.r = (unsigned char)closestWall->color.x;
        result.g = (unsigned char)closestWall->color.y;
        result.b = (unsigned char)closestWall->color.z;

        float minX = closestWall->position.x - closestWall->scale.x / 2.0f;
        float maxX = closestWall->position.x + closestWall->scale.x / 2.0f;
        float minY = closestWall->position.y - closestWall->scale.y / 2.0f;
        float maxY = closestWall->position.y + closestWall->scale.y / 2.0f;

        float u;
        switch (result.side) {
            case 0:
            case 1: u = (ray->position.x - minX) / closestWall->scale.x; break;
            case 2:
            case 3: u = (ray->position.y - minY) / closestWall->scale.y; break;
        }
        result.u = u;

        return result;
    }

    ray->position.x = origin.x + dir.x * MAX_RAY_LENGTH;
    ray->position.y = origin.y + dir.y * MAX_RAY_LENGTH;

    return result;
}

