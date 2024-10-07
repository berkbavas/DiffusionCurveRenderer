#pragma once

namespace DiffusionCurveRenderer
{
    struct Mouse
    {
        float x{ 0 };
        float y{ 0 };
        float dx{ 0 };
        float dy{ 0 };

        Qt::MouseButton button{ Qt::NoButton };
    };
}