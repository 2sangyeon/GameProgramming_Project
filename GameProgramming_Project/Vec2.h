#pragma once
#include <cmath>

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& o) const
    {
        return { x + o.x, y + o.y };
    }

    Vec2 operator-(const Vec2& o) const
    {
        return{ x - o.x,y - o.y };
    }

    Vec2 operator*(float s) const
    {
        return { x * s, y * s };
    }

    Vec2& operator+=(const Vec2& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    Vec2 Normalized() const
    {
        float len = std::sqrt(x * x + y * y);
        return (len > 0.0f) ? Vec2{ x / len,y / len } : Vec2{ 0,0 };
    }
};