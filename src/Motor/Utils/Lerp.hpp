#pragma once
#include "Vector2D.hpp"

namespace CE
{
    Vector2D lerp(const Vector2D& init, const Vector2D& dest, float t);
    Vector2D lerp2(const Vector2D& init, const Vector2D& ctrl, const Vector2D& dest, float t);
}
