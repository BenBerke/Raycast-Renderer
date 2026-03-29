//
// Created by berke on 3/11/2026.
//

#include "../../Headers/Objects/Player.h"

#include <math.h>

void player_update(Player *player) {
    Vector2 velocity = {player->velocity.x, player->velocity.y};
    player_set_position(player, vector2_add(player->position, velocity));

    player->velocity.x *= player->friction;
    player->velocity.y *= player->friction;
}
void player_set_velocity(Player *player, Vector2 velocity) {
    player->velocity = velocity;
}
void player_set_position(Player *player, Vector2 position) {
    player->position = position;
}
void player_add_velocity(Player *player, Vector2 velocity) {
    player->velocity = vector2_add(player->velocity, velocity);
}

Vector2 player_get_facing_dir(const Player* player) {
    Vector2 dir = { cosf(player->angle), sinf(player->angle) };
    return dir;
}