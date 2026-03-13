#ifndef RAYCAST_RENDERER_TEXTUREMANAGER_H
#define RAYCAST_RENDERER_TEXTUREMANAGER_H

#include <SDL3/SDL.h>

typedef struct {
    SDL_Texture* texture;
    float width;
    float height;
} Texture;

bool create_texture(Texture* texture, const char* textureNameNoExt, SDL_Renderer* renderer);
void destroy_texture(Texture* texture);

#endif