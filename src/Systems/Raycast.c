//
// Created by berke on 3/12/2026.
//

#include <math.h>

#include "../../Headers/Systems/Raycast.h"

#include <stdlib.h>

#include "../../config.h"
#include "../../Headers/Systems/Physics.h"

RayReturn raycast_create_ray(Ray* ray, Player* p, const Vector2 dir, const WallsList* list) {
    RayReturn result = {-1.0f, 0, 0, 0};

    const Vector2 origin = p->position;

    float closestT = (float)MAX_RAY_LENGTH;
    const Wall* closestWall = NULL;

    for (int i = 0; i < list->count; i++) {
        float t;
        if (ray_intersect_wall(origin, dir, &list->items[i], &t)) {
            if (t >= 0.0f && t < closestT) {
                closestT = t;
                closestWall = &list->items[i];
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
        return result;
    }

    ray->position.x = origin.x + dir.x * MAX_RAY_LENGTH;
    ray->position.y = origin.y + dir.y * MAX_RAY_LENGTH;

    return result;
}

#include <math.h>
#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"

