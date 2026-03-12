//
// Created by berke on 3/12/2026.
//

#ifndef RAYCAST_RENDERER_PHYSICS_H
#define RAYCAST_RENDERER_PHYSICS_H

#include "../Objects/Player.h"
#include "../Objects/Wall.h"

void physics_check_collisions(Player* player, Wall* wall);

#endif //RAYCAST_RENDERER_PHYSICS_H