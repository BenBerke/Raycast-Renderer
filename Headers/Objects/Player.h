//
// Created by berke on 3/11/2026.
//

#ifndef RAYCAST_RENDERER_PLAYER_H
#define RAYCAST_RENDERER_PLAYER_H

#include "../Tools/Vector2.h"

typedef struct {
    Vector2 position;
    float scale;
    Vector2 velocity;

    float speed;
    float friction;

    float angle;
} Player;

void player_update(Player *player);
void player_set_velocity(Player *player, Vector2 velocity);
void player_set_position(Player *player, Vector2 position);
void player_add_velocity(Player *player, Vector2 velocity);

Vector2 player_get_facing_dir(const Player* player);

#endif //RAYCAST_RENDERER_PLAYER_H