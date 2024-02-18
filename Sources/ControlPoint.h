#pragma once

#include <QVector2D>

namespace DiffusionCurveRenderer
{
    class ControlPoint
    {
    public:
        ControlPoint();

        ControlPoint* DeepCopy() const;

    public:
        QVector2D mPosition;
        bool mSelected;
    };
}
