#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "../Objects/Ray.h"

typedef struct WallsList WallsList;

typedef struct {
    float distance;
    unsigned char r, g, b;
} RayReturn;

RayReturn raycast_create_ray(Ray* r, Player* p, Vector2 dir, const WallsList* list);

#endif