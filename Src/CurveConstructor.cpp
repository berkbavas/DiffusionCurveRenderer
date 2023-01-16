#include "CurveConstructor.h"

#include <QDebug>

DiffusionCurveRenderer::CurveConstructor::CurveConstructor()
    : mProgress(0.0f)
{}

void DiffusionCurveRenderer::CurveConstructor::Run(const QVector<QVector<Point>>& polylines)
{
    for (int i = 0; i < polylines.size(); i++)
    {
        mProgress = float(i) / (polylines.size() - 1);

        Bezier* curve = ConstructCurve(polylines.at(i));

        if (curve)
        {
            // Split curve into several curves because we support Beizer curves having at most 32 control points
            if (curve->GetControlPoints().size() >= 29)
            {
                QList<ControlPoint*> controlPoints = curve->GetControlPoints();

                QList<Bezier*> newCurves;
                newCurves << new Bezier;
                newCurves.last()->AddControlPoint(controlPoints.at(0)->DeepCopy());
                newCurves.last()->AddControlPoint(controlPoints.at(1)->DeepCopy());

                for (int i = 2; i < controlPoints.size() - 2; i++)
                {
                    newCurves.last()->AddControlPoint(controlPoints.at(i)->DeepCopy());

                    if (newCurves.last()->GetControlPoints().size() == 29)
                    {
                        newCurves.last()->RemoveControlPoint(28);
                        i--;
                        newCurves << new Bezier;
                        newCurves.last()->AddControlPoint(controlPoints.at(i)->DeepCopy());
                        newCurves.last()->AddControlPoint(controlPoints.at(i + 1)->DeepCopy());
                    }
                }

                newCurves.last()->AddControlPoint(controlPoints.at(controlPoints.size() - 2)->DeepCopy());
                newCurves.last()->AddControlPoint(controlPoints.at(controlPoints.size() - 1)->DeepCopy());

                mCurves << newCurves;

                delete curve;
            }
            else
            {
                mCurves << curve;
            }
        }
    }
}

DiffusionCurveRenderer::Bezier* DiffusionCurveRenderer::CurveConstructor::ConstructCurve(const QVector<Point>& polyline, double tension)
{
    const int nPoints = polyline.size();

    if (nPoints <= 1)
    {
        qInfo() << "Number of points in the polyline is less than 1. Number of points is" << nPoints;
        return nullptr;
    }

    Bezier* curve = new Bezier;

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

    ControlPoint* firstHandle = polyline.at(0).ToControlPoint();
    ControlPoint* firstControlHandle = Point(firstControl(0), firstControl(1)).ToControlPoint();

    curve->AddControlPoint(firstHandle);
    curve->AddControlPoint(firstControlHandle);

    for (int i = 1; i < nPoints - 1; i++)
    {
        Eigen::Vector2f curr = polyline.at(i).ToVector();
        Eigen::Vector2f currDerivative = derivatives.col(i);

        Eigen::Vector2f prevControl = curr - currDerivative / 3.0;
        Eigen::Vector2f nextControl = curr + currDerivative / 3.0;

        Point currHandle = polyline.at(i);
        Point prevControlHandle(prevControl(0), prevControl(1));
        Point nextControlHandle(nextControl(0), nextControl(1));

        curve->AddControlPoint(prevControlHandle.ToControlPoint());
        curve->AddControlPoint(currHandle.ToControlPoint());
        curve->AddControlPoint(nextControlHandle.ToControlPoint());
    }

    // Correct the normal around the first handle.
    Eigen::Vector2f lastDerivative = derivatives.col(nPoints - 1);
    Eigen::Vector2f lastControl = last - lastDerivative / 3.0;

    Point lastHandle = polyline.at(nPoints - 1);
    Point lastControlHandle(lastControl(0), lastControl(1));

    curve->AddControlPoint(lastControlHandle.ToControlPoint());
    curve->AddControlPoint(lastHandle.ToControlPoint());

    return curve;
}

const QVector<DiffusionCurveRenderer::Bezier*>& DiffusionCurveRenderer::CurveConstructor::GetCurves() const
{
    return mCurves;
}

float DiffusionCurveRenderer::CurveConstructor::GetProgress() const
{
    return mProgress;
}