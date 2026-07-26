#include "BezierCurveConstructor.h"

#include <QDebug>

DiffusionCurveRenderer::BezierCurveConstructor::BezierCurveConstructor(QObject* pParent)
    : VectorizationStageBase(pParent)
{}

void DiffusionCurveRenderer::BezierCurveConstructor::Run(const QVector<QVector<Point>>& Polylines)
{
    constexpr int NUMBER_OF_POINTS_PER_POLYLINE = 8;
    const auto nPolylines = Polylines.size();

    for (int I = 0; I < nPolylines; I++)
    {
        const auto& Polyline = Polylines.at(I);
        const auto nPoints = Polyline.size();

        if (nPoints > NUMBER_OF_POINTS_PER_POLYLINE)
        {
            const auto NumberOfSmallerSegments = nPoints / NUMBER_OF_POINTS_PER_POLYLINE + 1;
            for (int IndexOfSmallerSegments = 0; IndexOfSmallerSegments < NumberOfSmallerSegments; ++IndexOfSmallerSegments)
            {
                QVector<Point> Smaller;

                for (int j = 0; j < NUMBER_OF_POINTS_PER_POLYLINE; j++)
                {
                    const auto IndexOfPoint = NUMBER_OF_POINTS_PER_POLYLINE * IndexOfSmallerSegments + j;
                    if (IndexOfPoint >= Polyline.size())
                    {
                        if (Smaller.size() == 1)
                        {
                            Smaller.append(Polyline.at(IndexOfPoint - 2));
                        }
                        break;
                    }

                    Smaller.append(Polyline.at(IndexOfPoint));
                }

                if (CurvePtr pCurve = ConstructCurve(Smaller))
                {
                    mCurves << pCurve;
                }
            }
        }
        else if (CurvePtr pCurve = ConstructCurve(Polyline))
        {
            mCurves << pCurve;
        }

        emit ProgressChanged(float(I) / (nPolylines - 1));
    }

    emit Finished();
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::BezierCurveConstructor::ConstructCurve(const QVector<Point>& Polyline, double Tension)
{
    const int nPoints = Polyline.size();

    if (nPoints <= 1)
    {
        qInfo() << "Number of points in the polyline is less than 1. Number of points is" << nPoints;
        return nullptr;
    }

    CurvePtr pCurve = std::make_shared<Bezier>();

    Eigen::Vector2f First = Polyline.at(0).ToVector();
    Eigen::Vector2f Second = Polyline.at(1).ToVector();
    Eigen::Vector2f SecondLast = Polyline.at(nPoints - 2).ToVector();
    Eigen::Vector2f Last = Polyline.at(nPoints - 1).ToVector();

    Eigen::MatrixXf Derivatives(2, nPoints);
    Derivatives.col(0) = (Second - First) / Tension;
    Derivatives.col(nPoints - 1) = (Last - SecondLast) / Tension;

    for (int I = 1; I < Polyline.size() - 1; I++)
    {
        Eigen::Vector2f Next = Polyline.at(I + 1).ToVector();
        Eigen::Vector2f Prev = Polyline.at(I - 1).ToVector();

        Derivatives.col(I) = (Next - Prev) / Tension;
    }

    Eigen::Vector2f FirstDerivative = Derivatives.col(0);
    Eigen::Vector2f FirstControl = First + FirstDerivative / 3.0;

    pCurve->AddControlPoint(QVector2D(Polyline.at(0).X, Polyline.at(0).Y));
    pCurve->AddControlPoint(QVector2D(FirstControl(0), FirstControl(1)));

    for (int I = 1; I < nPoints - 1; I++)
    {
        Eigen::Vector2f Curr = Polyline.at(I).ToVector();
        Eigen::Vector2f CurrDerivative = Derivatives.col(I);

        Eigen::Vector2f PrevControl = Curr - CurrDerivative / 3.0;
        Eigen::Vector2f NextControl = Curr + CurrDerivative / 3.0;

        Point CurrHandle = Polyline.at(I);
        Point PrevControlHandle(PrevControl(0), PrevControl(1));
        Point NextControlHandle(NextControl(0), NextControl(1));

        pCurve->AddControlPoint(QVector2D(PrevControlHandle.X, PrevControlHandle.Y));
        pCurve->AddControlPoint(QVector2D(CurrHandle.X, CurrHandle.Y));
        pCurve->AddControlPoint(QVector2D(NextControlHandle.X, NextControlHandle.Y));
    }

    // Correct the normal around the first handle.
    Eigen::Vector2f LastDerivative = Derivatives.col(nPoints - 1);
    Eigen::Vector2f LastControl = Last - LastDerivative / 3.0;

    Point LastHandle = Polyline.at(nPoints - 1);
    Point LastControlHandle(LastControl(0), LastControl(1));

    pCurve->AddControlPoint(QVector2D(LastControlHandle.X, LastControlHandle.Y));
    pCurve->AddControlPoint(QVector2D(LastHandle.X, LastHandle.Y));

    return pCurve;
}

const QVector<DiffusionCurveRenderer::CurvePtr>& DiffusionCurveRenderer::BezierCurveConstructor::GetCurves() const
{
    return mCurves;
}

void DiffusionCurveRenderer::BezierCurveConstructor::Reset()
{
    mCurves.clear();
}