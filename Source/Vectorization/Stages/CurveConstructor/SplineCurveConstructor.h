#pragma once

#include "Curve/Spline.h"
#include "Vectorization/Stages/Base/VectorizationStageBase.h"
#include "Vectorization/Stages/CurveConstructor/CurveConstructor.h"

namespace DiffusionCurveRenderer
{
    class SplineCurveConstructor : public VectorizationStageBase, public CurveConstructor
    {
      public:
        explicit SplineCurveConstructor(QObject* parent);

        void Run(const QVector<QVector<Point>>& polylines);

        const QVector<CurvePtr>& GetCurves() const override;

        void Reset() override;

      private:
        CurvePtr ConstructCurve(const QVector<Point>& polyline, double tension = 2.0);

      private:
        QVector<CurvePtr> mCurves;
    };
}
