//
// Created by berke on 3/14/2026.
//

#ifndef RAYCAST_RENDERER_SPRITE_H
#define RAYCAST_RENDERER_SPRITE_H

#include "../Tools/Vector2.h"
#include "../Tools/Vector3.h"
#include "../Systems/TextureManager.h"

typedef struct {
    Vector2 playerToObject;
    float distanceToPlayer;
    float angleFromPlayerToObject;
} ReturnInformation;

typedef struct {
    Vector2 position;
    Vector2 scale;
    Vector3 color;
    int texture;
} Object;

typedef struct {
    Object* items;
    int count;
    int size;
    int chunkSize;
} ObjectsList;

void objects_create_objects_list(ObjectsList* list, int chunkSize);
void objects_push_objects_list(ObjectsList* list, Object* value);
void objects_pop_objects_list(ObjectsList* list);
void objects_free_objects_list(ObjectsList* list);

#endif //RAYCAST_RENDERER_SPRITE_H