#include "Camera2D.h"

Camera2D::Camera2D()
{
}

void Camera2D::FollowSmooth(float targetX, float targetY, float dt)
{
    float desiredX = targetX - (screenW / zoom) * 0.5f;
    float desiredY = targetY - (screenH / zoom) * 0.5f;

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
        static_cast<int>((worldRect.x - x) * zoom),
        static_cast<int>((worldRect.y - y) * zoom),
        static_cast<int>(worldRect.w * zoom),
        static_cast<int>(worldRect.h * zoom)
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

float Camera2D::GetZoom() const
{
    return zoom;
}