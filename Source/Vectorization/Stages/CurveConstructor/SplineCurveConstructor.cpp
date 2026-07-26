#include "SplineCurveConstructor.h"

#include <QDebug>

DiffusionCurveRenderer::SplineCurveConstructor::SplineCurveConstructor(QObject* pParent)
    : VectorizationStageBase(pParent)
{}

void DiffusionCurveRenderer::SplineCurveConstructor::Run(const QVector<QVector<Point>>& Polylines)
{
    for (int i = 0; i < Polylines.size(); i++)
    {
        if (CurvePtr pCurve = ConstructCurve(Polylines.at(i)))
        {
            mCurves << pCurve;
        }

        const auto Progress = float(i) / (Polylines.size() - 1);
        emit ProgressChanged(Progress);
    }
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::SplineCurveConstructor::ConstructCurve(const QVector<Point>& Polyline, double Tension)
{
    const int nPoints = Polyline.size();

    if (nPoints <= 1)
    {
        qInfo() << "Number of points in the polyline is less than or equal to 1. Number of points is" << nPoints;
        return nullptr;
    }

    SplinePtr pSpline = std::make_shared<Spline>();

    for (int I = 0; I < nPoints; I++)
    {
        const auto Point = Polyline.at(I);
        pSpline->AddControlPoint(QVector2D(Point.X, Point.Y));
    }

    return static_cast<CurvePtr>(pSpline);
}

const QVector<DiffusionCurveRenderer::CurvePtr>& DiffusionCurveRenderer::SplineCurveConstructor::GetCurves() const
{
    return mCurves;
}

void DiffusionCurveRenderer::SplineCurveConstructor::Reset()
{
    mCurves.clear();
}
