//
// Created by berke on 3/12/2026.
//

#include "../../Headers/Systems/Physics.h"

#include <float.h>
#include <math.h>

#include "../../config.h"

#include <stdlib.h>

#include "../../Headers/Systems/Raycast.h"

void physics_create_walls_list(WallsList* list, int chunkSize){
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc(list->chunkSize * sizeof(*(list->items)));
}

void physics_push_walls_list(WallsList* list, Wall* value){
    if(++list->count > list->size){
        list->items = realloc(list->items, (list->size+=list->chunkSize)*sizeof(*(list->items)));
    }
    list->items[list->count-1] = *value;
}

void physics_pop_walls_list(WallsList* list){
    if(list->size - (--list->count) >= list->chunkSize && list->size > list->chunkSize){
        list->size -= list->chunkSize;
        list->items = realloc(list->items, list->size*sizeof(*(list->items)));
    }
}

void physics_free_walls_list(WallsList* list) {
    free(list->items);
}

void physics_check_collisions(Player* player, const WallsList* list) {
    const float half = player->scale / 2.0f;
    const float epsilon = 0.001f;

    for (int i = 0; i < list->count; i++) {
        const Wall* wall = &list->items[i];

        const float px = player->position.x;
        const float py = player->position.y;

        const float wx = wall->position.x;
        const float wy = wall->position.y;
        const float w_half_x = wall->scale.x / 2.0f;
        const float w_half_y = wall->scale.y / 2.0f;

        if (px + half > wx - w_half_x &&
            px - half < wx + w_half_x &&
            py + half > wy - w_half_y &&
            py - half < wy + w_half_y) {

            float overlapX = (px + half) - (wx - w_half_x);
            float overlapX2 = (wx + w_half_x) - (px - half);
            if (overlapX2 < overlapX) overlapX = overlapX2;

            float overlapY = (py + half) - (wy - w_half_y);
            float overlapY2 = (wy + w_half_y) - (py - half);
            if (overlapY2 < overlapY) overlapY = overlapY2;

            if (overlapX < overlapY) {
                if (px > wx)
                    player->position.x += overlapX + epsilon;
                else
                    player->position.x -= overlapX + epsilon;

                player->velocity.x = 0;
            } else {
                if (py > wy)
                    player->position.y += overlapY + epsilon;
                else
                    player->position.y -= overlapY + epsilon;

                player->velocity.y = 0;
            }
            }
    }
}

bool ray_intersect_wall(Vector2 origin, Vector2 dir, const Wall* wall, float* outT) {
    const float minX = wall->position.x - wall->scale.x / 2.0f;
    const float maxX = wall->position.x + wall->scale.x / 2.0f;
    const float minY = wall->position.y - wall->scale.y / 2.0f;
    const float maxY = wall->position.y + wall->scale.y / 2.0f;

    float tMin = -FLT_MAX;
    float tMax = FLT_MAX;

    if (fabsf(dir.x) < 0.00001f) {
        if (origin.x < minX || origin.x > maxX) return false;
    } else {
        float tx1 = (minX - origin.x) / dir.x;
        float tx2 = (maxX - origin.x) / dir.x;

        if (tx1 > tx2) {
            float temp = tx1;
            tx1 = tx2;
            tx2 = temp;
        }

        if (tx1 > tMin) tMin = tx1;
        if (tx2 < tMax) tMax = tx2;
    }

    if (fabsf(dir.y) < 0.00001f) {
        if (origin.y < minY || origin.y > maxY) return false;
    } else {
        float ty1 = (minY - origin.y) / dir.y;
        float ty2 = (maxY - origin.y) / dir.y;

        if (ty1 > ty2) {
            float temp = ty1;
            ty1 = ty2;
            ty2 = temp;
        }

        if (ty1 > tMin) tMin = ty1;
        if (ty2 < tMax) tMax = ty2;
    }

    if (tMin > tMax) return false;
    if (tMax < 0.0f) return false;

    *outT = (tMin >= 0.0f) ? tMin : tMax;
    return true;
}

void* physics_check_ray_hit(Ray* ray, const WallsList* list) {
    for (int i = 0; i < list->count; i++) {
        const float half = RAY_SIZE;
        const Wall* wall = &list->items[i];

        const float px = ray->position.x;
        const float py = ray->position.y;

        const float wx = wall->position.x;
        const float wy = wall->position.y;
        const float w_half_x = wall->scale.x / 2.0f;
        const float w_half_y = wall->scale.y / 2.0f;

        if (px + half > wx - w_half_x &&
            px - half < wx + w_half_x &&
            py + half > wy - w_half_y &&
            py - half < wy + w_half_y) {
            return &list->items[i];
        }
    }
    return NULL;
}

