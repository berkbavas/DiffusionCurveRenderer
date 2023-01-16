#pragma once

#include "Common.h"
#include <QVector2D>

namespace DiffusionCurveRenderer
{
    class Bezier;

    class BlurPoint
    {
    public:
        BlurPoint();

        void SetParent(Bezier* newParent);
        QVector2D GetPosition2D(float gap = BLUR_POINT_VISUAL_GAP) const;

    public:
        float mStrength;
        float mPosition;
        bool mSelected;

    private:
        Bezier* mParent;
    };
}
