#pragma once

#include <QObject>

namespace DiffusionCurveRenderer
{
    struct Mouse
    {
        float X{ 0 };
        float Y{ 0 };
        float Dx{ 0 };
        float Dy{ 0 };

        Qt::MouseButton Button{ Qt::NoButton };
    };
}