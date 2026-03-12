//
// Created by berke on 3/12/2026.
//

#include "../../Headers/Systems/Physics.h"

#include <stdio.h>

void physics_check_collisions(Player* player, Wall* wall) {
    const float px = player->position.x, py = player->position.y, ps = player->scale/2;
    const float wx = wall->position.x, wy = wall->position.y, wsx = wall->scale.x/2, wsy = wall->scale.y/2;

    const float playerLeft = px - ps;
    const float playerRight = px + ps;
    const float playerTop = py + ps;
    const float playerBottom = py - ps;

    const float wallLeft = wx - wsx;
    const float wallRight = wx + wsx;
    const float wallTop = wy + wsy;
    const float wallBottom = wy - wsy;

    if (playerLeft < wallRight && playerRight > wallLeft && playerTop > wallBottom && playerBottom < wallTop)
    {
        float overlapLeft = playerRight - wallLeft;
        float overlapRight = wallRight - playerLeft;
        float overlapTop = wallTop - playerBottom;
        float overlapBottom = playerTop - wallBottom;

        float minOverlap = overlapLeft;

        if (overlapRight < minOverlap) minOverlap = overlapRight;
        if (overlapTop < minOverlap) minOverlap = overlapTop;
        if (overlapBottom < minOverlap) minOverlap = overlapBottom;

        if (minOverlap == overlapLeft)
        {
            player->position.x -= overlapLeft;
            player->velocity.x = 0;
        }
        else if (minOverlap == overlapRight)
        {
            player->position.x += overlapRight;
            player->velocity.x = 0;
        }
        else if (minOverlap == overlapTop)
        {
            player->position.y += overlapTop;
            player->velocity.y = 0;
        }
        else if (minOverlap == overlapBottom)
        {
            player->position.y -= overlapBottom;
            player->velocity.y = 0;
        }
    }
}
