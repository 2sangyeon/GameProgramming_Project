#include "Camera2D.h"

Camera2D::Camera2D()
{
}

void Camera2D::FollowSmooth(float targetX, float targetY, float dt)
{
    float desiredX = targetX - screenW * 0.5f;
    float desiredY = targetY - screenH * 0.5f;

    x += (desiredX - x) * smoothing * dt;
    y += (desiredY - y) * smoothing * dt;
}

void Camera2D::ClampToBounds(float mapWidth, float mapHeight)
{
    if (x < 0.0f) x = 0.0f;
    if (y < 0.0f) y = 0.0f;

    if (x > mapWidth - screenW)
        x = mapWidth - screenW;

    if (y > mapHeight - screenH)
        y = mapHeight - screenH;
}

SDL_Rect Camera2D::WorldToScreen(const SDL_Rect& worldRect) const
{
    return {
        worldRect.x - static_cast<int>(x),
        worldRect.y - static_cast<int>(y),
        worldRect.w,
        worldRect.h
    };
}

float Camera2D::GetX() const
{
    return x;
}

float Camera2D::GetY() const
{
    return y;
}