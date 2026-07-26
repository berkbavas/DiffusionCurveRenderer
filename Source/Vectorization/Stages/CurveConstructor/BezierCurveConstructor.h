#pragma once

#include "Curve/Bezier.h"
#include "Vectorization/Stages/Base/VectorizationStageBase.h"
#include "Vectorization/Stages/CurveConstructor/CurveConstructor.h"

namespace DiffusionCurveRenderer
{
    class BezierCurveConstructor : public VectorizationStageBase, public CurveConstructor
    {
      public:
        explicit BezierCurveConstructor(QObject* pParent);

        void Run(const QVector<QVector<Point>>& Polylines);
        const QVector<CurvePtr>& GetCurves() const override;
        void Reset() override;

      private:
        CurvePtr ConstructCurve(const QVector<Point>& Polyline, double Tension = 2.0);

      private:
        QVector<CurvePtr> mCurves;
    };
}
