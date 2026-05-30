#include "BackGround.h"
#include <SDL_image.h>

bool BackGround::Load(SDL_Renderer* renderer, const char* path)
{
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) return false;

    width = surface->w;
    height = surface->h;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    return texture != nullptr;
}

void BackGround::Render(SDL_Renderer* renderer, const Camera2D& camera)
{
    if (!texture) return;

    SDL_Rect dst =
    {
        -static_cast<int>(camera.GetX() * parallaxFactor),
        -static_cast<int>(camera.GetY() * parallaxFactor),
        width,
        height
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dst);
}

BackGround::~BackGround()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}