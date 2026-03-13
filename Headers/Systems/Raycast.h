#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "../Objects/Ray.h"

typedef struct WallsList WallsList;

typedef struct {
    float distance;
    unsigned char r, g, b;
    char side; // 0 North, 1 South, 2 West, 3 East
    float u;
} RayReturn;

RayReturn raycast_create_ray(Ray* r, Player* p, Vector2 dir, const WallsList* list);

#endif