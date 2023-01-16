#pragma once

#include "Common.h"
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class Bezier;

    class ColorPoint
    {
    public:
        ColorPoint();

        void SetParent(Bezier* newParent);
        QVector2D GetPosition2D(float gap = COLOR_POINT_VISUAL_GAP) const;

        enum class Direction
        {
            Left,
            Right
        };

    public:
        QVector4D mColor;
        float mPosition;
        bool mSelected;
        Direction mDirection;

    private:
        Bezier* mParent;
    };
}
