#ifndef RAYCAST_RENDERER_RAYCAST_H
#define RAYCAST_RENDERER_RAYCAST_H

#include "../Tools/Vector2.h"
#include "../Objects/Player.h"
#include "../Objects/Ray.h"
#include "Physics.h"

typedef struct {
    float distance;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    char side;   /* 0 top, 1 bottom, 2 left, 3 right */
    float u;     /* 0.0 -> 1.0 across the wall face */
    int texture;
} RayReturn;

RayReturn raycast_create_ray(Ray* ray, const Player* player, Vector2 dir, const WallsList* list);

#endif