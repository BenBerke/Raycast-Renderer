//
// Created by berke on 3/12/2026.
//

#include <math.h>

#include "../../Headers/Systems/Raycast.h"

#include "../../config.h"
#include "../../Headers/Systems/Physics.h"

float raycast_create_ray(Ray* r, Player* p, const Vector2 dir, const WallsList* list) {
    r->position.x = p->position.x; r->position.y = p->position.y;
    int i = 0;
    while (!physics_check_ray_hit(r, list)) {
        raycast_move_ray(r, dir, 1);
        if (i > MAX_RAY_LENGTH) return -1;
        i++;
    }
    return sqrtf((p->position.x - r->position.x) * (p->position.x - r->position.x) + (p->position.y - r->position.y) * (p->position.y - r->position.y));
}

void raycast_move_ray(Ray* r, const Vector2 dir, const float speed) {
    r->position = vector2_add(r->position, vector2_multiply_with_float(dir, speed));
}

#include <math.h>
#include "../../config.h"
#include "../../Headers/Systems/Renderer.h"

