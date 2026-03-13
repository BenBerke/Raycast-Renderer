#include "../../Headers/Systems/TextureManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3_image/SDL_image.h>

void destroy_texture(Texture* texture) {
    if (texture->texture != NULL) {
        SDL_DestroyTexture(texture->texture);
        texture->texture = NULL;
    }

    texture->width = 0.0f;
    texture->height = 0.0f;
}

void textureManager_create_textures_list(TexturesList* list, const int chunkSize) {
    list->count = 0;
    list->size = chunkSize;
    list->chunkSize = chunkSize;
    list->items = malloc((size_t)list->size * sizeof(*list->items));
}

void textureManager_push_textures_list(TexturesList* list, const Texture* value) {
    if (++list->count > list->size) {
        list->size += list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
    list->items[list->count - 1] = *value;
}

void textureManager_pop_textures_list(TexturesList* list) {
    if (list->count <= 0) {
        return;
    }

    list->count--;

    if (list->size - list->count >= list->chunkSize && list->size > list->chunkSize) {
        list->size -= list->chunkSize;
        list->items = realloc(list->items, (size_t)list->size * sizeof(*list->items));
    }
}

void textureManager_free_walls_list(TexturesList* list) {
    for (int i = 0; i < list->count; i++) SDL_DestroyTexture(list->items[i].texture);
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->size = 0;
    list->chunkSize = 0;
}

int create_texture(const char* textureNameNoExt, TexturesList* textureList, SDL_Renderer* renderer) {
    Texture texture = {0};
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "Assets/%s.png", textureNameNoExt);

    texture.texture = IMG_LoadTexture(renderer, fullPath);
    if (texture.texture == NULL) {
        SDL_Log("IMG_LoadTexture %s failed: %s", fullPath, SDL_GetError());
        texture.width = 0.0f;
        texture.height = 0.0f;
        return -1;
    }

    SDL_GetTextureSize(texture.texture, &texture.width, &texture.height);
    SDL_SetTextureScaleMode(texture.texture, SDL_SCALEMODE_NEAREST);
    textureManager_push_textures_list(textureList, &texture);
    return textureList->count-1;
}