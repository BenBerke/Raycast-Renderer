#include "../../Headers/Systems/Physics.h"

#include <stdlib.h>

void physics_create_walls_list(WallsList* list, int chunkSize) {
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc((size_t)list->size * sizeof(*list->items));
}

void physics_push_walls_list(WallsList* list, Wall* value) {
    if (++list->count > list->size) {
        list->size += list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
    list->items[list->count - 1] = *value;
}

void physics_pop_walls_list(WallsList* list) {
    if (list->count <= 0) {
        return;
    }

    list->count--;

    if (list->size - list->count >= list->chunkSize && list->size > list->chunkSize) {
        list->size -= list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
}

void physics_free_walls_list(WallsList* list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->size = 0;
    list->chunkSize = 0;
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
        const float wHalfX = wall->scale.x / 2.0f;
        const float wHalfY = wall->scale.y / 2.0f;

        if (px + half > wx - wHalfX &&
            px - half < wx + wHalfX &&
            py + half > wy - wHalfY &&
            py - half < wy + wHalfY) {

            float overlapX = (px + half) - (wx - wHalfX);
            float overlapX2 = (wx + wHalfX) - (px - half);
            if (overlapX2 < overlapX) {
                overlapX = overlapX2;
            }

            float overlapY = (py + half) - (wy - wHalfY);
            float overlapY2 = (wy + wHalfY) - (py - half);
            if (overlapY2 < overlapY) {
                overlapY = overlapY2;
            }

            if (overlapX < overlapY) {
                if (px > wx) {
                    player->position.x += overlapX + epsilon;
                } else {
                    player->position.x -= overlapX + epsilon;
                }
                player->velocity.x = 0.0f;
            } else {
                if (py > wy) {
                    player->position.y += overlapY + epsilon;
                } else {
                    player->position.y -= overlapY + epsilon;
                }
                player->velocity.y = 0.0f;
            }
        }
    }
}