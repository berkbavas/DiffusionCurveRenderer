#pragma once

#include "Bezier.h"
#include "Point.h"

namespace DiffusionCurveRenderer
{
    class CurveConstructor
    {
    public:
        CurveConstructor();

        void Run(const QVector<QVector<Point>>& polylines);

        const QVector<Bezier*>& GetCurves() const;
        float GetProgress() const;

    private:
        Bezier* ConstructCurve(const QVector<Point>& polyline, double tension = 2.0);

    private:
        QVector<Bezier*> mCurves;
        float mProgress;
    };
}
