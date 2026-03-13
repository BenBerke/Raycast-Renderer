#include "../../Headers/Systems/TextureManager.h"

#include <stdio.h>
#include <string.h>

#include <SDL3_image/SDL_image.h>

bool create_texture(Texture* texture, const char* textureNameNoExt, SDL_Renderer* renderer) {
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "Assets/%s.png", textureNameNoExt);

    texture->texture = IMG_LoadTexture(renderer, fullPath);
    if (texture->texture == NULL) {
        SDL_Log("IMG_LoadTexture %s failed: %s", fullPath, SDL_GetError());
        texture->width = 0.0f;
        texture->height = 0.0f;
        return false;
    }

    SDL_GetTextureSize(texture->texture, &texture->width, &texture->height);
    SDL_SetTextureScaleMode(texture->texture, SDL_SCALEMODE_NEAREST);
    return true;
}

void destroy_texture(Texture* texture) {
    if (texture->texture != NULL) {
        SDL_DestroyTexture(texture->texture);
        texture->texture = NULL;
    }

    texture->width = 0.0f;
    texture->height = 0.0f;
}