#pragma once

#include "Curve/Spline.h"
#include "Vectorization/Stages/Base/Point.h"
#include "Vectorization/Stages/Base/VectorizationStageBase.h"

namespace DiffusionCurveRenderer
{
    class CurveConstructor : public VectorizationStageBase
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
