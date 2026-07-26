#pragma once

#include "Curve/Spline.h"
#include "Vectorization/Stages/Base/VectorizationStageBase.h"
#include "Vectorization/Stages/CurveConstructor/CurveConstructor.h"

namespace DiffusionCurveRenderer
{
    class SplineCurveConstructor : public VectorizationStageBase, public CurveConstructor
    {
      public:
        explicit SplineCurveConstructor(QObject* pParent);

        void Run(const QVector<QVector<Point>>& Polylines);
        const QVector<CurvePtr>& GetCurves() const override;
        void Reset() override;

      private:
        CurvePtr ConstructCurve(const QVector<Point>& Polyline, double Tension = 2.0);

      private:
        QVector<CurvePtr> mCurves;
    };
}
