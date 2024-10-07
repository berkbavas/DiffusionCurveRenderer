#pragma once

#include "Curve/Curve.h"
#include "Vectorization/Stages/Base/Point.h"

namespace DiffusionCurveRenderer
{
    class CurveConstructor
    {
      public:
        virtual void Run(const QVector<QVector<Point>>& polylines) = 0;
        virtual const QVector<CurvePtr>& GetCurves() const = 0;
    };
}
