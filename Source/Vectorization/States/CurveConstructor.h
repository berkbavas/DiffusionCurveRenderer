#pragma once

#include "Curve/Spline.h"
#include "Point.h"
#include "Vectorization/States/VectorizationStateBase.h"

namespace DiffusionCurveRenderer
{
    class CurveConstructor : public VectorizationStateBase
    {
      public:
        explicit CurveConstructor(QObject* parent);

        void Run(const QVector<QVector<Point>>& polylines);

        const QVector<CurvePtr>& GetCurves() const;

        void Reset() override;

      private:
        CurvePtr ConstructCurve(const QVector<Point>& polyline, double tension = 2.0);

      private:
        QVector<CurvePtr> mCurves;
    };
}
