#include "CurveContainer.h"

#include "Util/Chronometer.h"
#include "Util/Logger.h"

void DiffusionCurveRenderer::CurveContainer::AddCurve(CurvePtr pCurve)
{
    mCurves << pCurve;
}

void DiffusionCurveRenderer::CurveContainer::AddCurves(QList<CurvePtr> pCurves)
{
    mCurves.append(pCurves);
}

void DiffusionCurveRenderer::CurveContainer::RemoveCurve(CurvePtr pCurve)
{
    mCurves.removeAll(pCurve);
}

void DiffusionCurveRenderer::CurveContainer::Clear()
{
    mCurves.clear();
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::CurveContainer::GetCurve(int Index)
{
    DCR_ASSERT(0 <= Index && Index < mCurves.size());
    return mCurves.at(Index);
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::CurveContainer::GetCurveAround(const QVector2D& Test, float Radius)
{
    MEASURE_CALL_TIME(CURVE_CONTAINER_GET_CURVE_AROUND);

    float MinDistance = std::numeric_limits<float>::infinity();

    CurvePtr pResult = nullptr;

    for (const auto& pCurve : mCurves)
    {
        const float Distance = pCurve->GetDistanceToPoint(Test);

        if (Distance < MinDistance)
        {
            pResult = pCurve;
            MinDistance = Distance;
        }
    }

    if (Radius < MinDistance)
    {
        pResult = nullptr;
    }

    return pResult;
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalContourThickness(float Value)
{
    mGlobalContourThickness = Value;

    for (const auto& pCurve : mCurves)
    {
        pCurve->SetContourThickness(mGlobalContourThickness);
    }
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalDiffusionWidth(float Value)
{
    mGlobalDiffusionWidth = Value;

    for (const auto& pCurve : mCurves)
    {
        pCurve->SetDiffusionWidth(mGlobalDiffusionWidth);
    }
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalDiffusionGap(float Value)
{
    mGlobalDiffusionGap = Value;

    for (const auto& pCurve : mCurves)
    {
        pCurve->SetDiffusionGap(mGlobalDiffusionGap);
    }
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalBlurStrength(float Value)
{
    mGlobalBlurStrength = Value;

    for (const auto& pCurve : mCurves)
    {
        if (BezierPtr pBezier = std::dynamic_pointer_cast<Bezier>(pCurve))
        {
            pBezier->SetAllBlurPointsStrength(mGlobalBlurStrength);
        }
    }
}
