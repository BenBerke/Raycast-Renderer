#ifndef RAYCAST_RENDERER_PHYSICS_H
#define RAYCAST_RENDERER_PHYSICS_H

#include "../Objects/Player.h"
#include "../Objects/Wall.h"
#include "../Objects/Object.h"

typedef struct WallsList {
    Wall* items;
    int count;
    int size;
    int chunkSize;
} WallsList;

void physics_create_walls_list(WallsList* list, int chunkSize);
void physics_push_walls_list(WallsList* list, Wall* value);
void physics_pop_walls_list(WallsList* list);
void physics_free_walls_list(WallsList* list);

void physics_check_collisions(Player* player, const WallsList* list);

#endif