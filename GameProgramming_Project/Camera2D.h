#pragma once

#include <SDL.h>
#include "Constants.h"

class Camera2D
{
public:
    Camera2D();

    void FollowSmooth(float targetX, float targetY, float dt);
    void ClampToBounds(float mapWidth, float mapHeight);

    SDL_Rect WorldToScreen(const SDL_Rect& worldRect) const;

    float GetX() const;
    float GetY() const;

    float GetZoom() const;

private:
    float x = 0.0f;
    float y = 0.0f;

    int screenW = SCREEN_WIDTH;
    int screenH = SCREEN_HEIGHT;

    float smoothing = 2.0f;

    // 카메라 확대 배율
    float zoom = 2.0f;
};