#ifndef RAYCAST_RENDERER_TEXTUREMANAGER_H
#define RAYCAST_RENDERER_TEXTUREMANAGER_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Texture* texture;
    float width;
    float height;
} Texture;

typedef struct {
    Texture* items;
    int count;
    int size;
    int chunkSize;
} TexturesList;

void textureManager_create_textures_list(TexturesList* list, int chunkSize);
void textureManager_push_textures_list(TexturesList* list, const Texture* value);
void textureManager_pop_textures_list(TexturesList* list);
void textureManager_free_walls_list(TexturesList* list);

int create_texture(const char* textureNameNoExt, TexturesList* textureList, SDL_Renderer* renderer);
void destroy_texture(Texture* texture);

#endif