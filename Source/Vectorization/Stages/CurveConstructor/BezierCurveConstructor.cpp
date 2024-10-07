#include "BezierCurveConstructor.h"

#include <QDebug>

DiffusionCurveRenderer::BezierCurveConstructor::BezierCurveConstructor(QObject* parent)
    : VectorizationStageBase(parent)
{
}

void DiffusionCurveRenderer::BezierCurveConstructor::Run(const QVector<QVector<Point>>& polylines)
{
    for (int i = 0; i < polylines.size(); i++)
    {
        float progress = float(i) / (polylines.size() - 1);
        emit ProgressChanged(progress);

        if (CurvePtr curve = ConstructCurve(polylines.at(i)))
        {
            mCurves << curve;
        }
    }

    emit Finished();
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::BezierCurveConstructor::ConstructCurve(const QVector<Point>& polyline, double tension)
{
    const int nPoints = polyline.size();

    if (nPoints <= 1)
    {
        qInfo() << "Number of points in the polyline is less than 1. Number of points is" << nPoints;
        return nullptr;
    }

    CurvePtr curve = std::make_shared<Bezier>();

    Eigen::Vector2f first = polyline.at(0).ToVector();
    Eigen::Vector2f second = polyline.at(1).ToVector();
    Eigen::Vector2f secondLast = polyline.at(nPoints - 2).ToVector();
    Eigen::Vector2f last = polyline.at(nPoints - 1).ToVector();

    Eigen::MatrixXf derivatives(2, nPoints);
    derivatives.col(0) = (second - first) / tension;
    derivatives.col(nPoints - 1) = (last - secondLast) / tension;

    for (int i = 1; i < polyline.size() - 1; i++)
    {
        Eigen::Vector2f next = polyline.at(i + 1).ToVector();
        Eigen::Vector2f prev = polyline.at(i - 1).ToVector();

        derivatives.col(i) = (next - prev) / tension;
    }

    Eigen::Vector2f firstDerivative = derivatives.col(0);
    Eigen::Vector2f firstControl = first + firstDerivative / 3.0;

    curve->AddControlPoint(QVector2D(polyline.at(0).x, polyline.at(0).y));
    curve->AddControlPoint(QVector2D(firstControl(0), firstControl(1)));

    for (int i = 1; i < nPoints - 1; i++)
    {
        Eigen::Vector2f curr = polyline.at(i).ToVector();
        Eigen::Vector2f currDerivative = derivatives.col(i);

        Eigen::Vector2f prevControl = curr - currDerivative / 3.0;
        Eigen::Vector2f nextControl = curr + currDerivative / 3.0;

        Point currHandle = polyline.at(i);
        Point prevControlHandle(prevControl(0), prevControl(1));
        Point nextControlHandle(nextControl(0), nextControl(1));

        curve->AddControlPoint(QVector2D(prevControlHandle.x, prevControlHandle.y));
        curve->AddControlPoint(QVector2D(currHandle.x, currHandle.y));
        curve->AddControlPoint(QVector2D(nextControlHandle.x, nextControlHandle.y));
    }

    // Correct the normal around the first handle.
    Eigen::Vector2f lastDerivative = derivatives.col(nPoints - 1);
    Eigen::Vector2f lastControl = last - lastDerivative / 3.0;

    Point lastHandle = polyline.at(nPoints - 1);
    Point lastControlHandle(lastControl(0), lastControl(1));

    curve->AddControlPoint(QVector2D(lastControlHandle.x, lastControlHandle.y));
    curve->AddControlPoint(QVector2D(lastHandle.x, lastHandle.y));

    return curve;
}

const QVector<DiffusionCurveRenderer::CurvePtr>& DiffusionCurveRenderer::BezierCurveConstructor::GetCurves() const
{
    return mCurves;
}

void DiffusionCurveRenderer::BezierCurveConstructor::Reset()
{
    mCurves.clear();
}