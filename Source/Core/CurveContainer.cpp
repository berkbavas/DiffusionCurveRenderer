#include "CurveContainer.h"

#include "Util/Chronometer.h"
#include "Util/Logger.h"

void DiffusionCurveRenderer::CurveContainer::AddCurve(CurvePtr curve)
{
    mCurves << curve;
}

void DiffusionCurveRenderer::CurveContainer::AddCurves(QList<CurvePtr> curves)
{
    mCurves.append(curves);
}

void DiffusionCurveRenderer::CurveContainer::RemoveCurve(CurvePtr curve)
{
    mCurves.removeAll(curve);
}

void DiffusionCurveRenderer::CurveContainer::Clear()
{
    mCurves.clear();
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::CurveContainer::GetCurve(int index)
{
    DCR_ASSERT(0 <= index && index < mCurves.size());
    return mCurves.at(index);
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::CurveContainer::GetCurveAround(const QVector2D& test, float radius)
{
    MEASURE_CALL_TIME(CURVE_CONTAINER_GET_CURVE_AROUND);

    float minDistance = std::numeric_limits<float>::infinity();

    CurvePtr result = nullptr;

    for (const auto& curve : mCurves)
    {
        const float distance = curve->GetDistanceToPoint(test);

        if (distance < minDistance)
        {
            result = curve;
            minDistance = distance;
        }
    }

    if (radius < minDistance)
    {
        result = nullptr;
    }

    return result;
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalContourThickness(float val)
{
    mGlobalContourThickness = val;

    for (const auto& curve : mCurves)
    {
        curve->SetContourThickness(mGlobalContourThickness);
    }
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalDiffusionWidth(float val)
{
    mGlobalDiffusionWidth = val;

    for (const auto& curve : mCurves)
    {
        curve->SetDiffusionWidth(mGlobalDiffusionWidth);
    }
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalDiffusionGap(float val)
{
    mGlobalDiffusionGap = val;

    for (const auto& curve : mCurves)
    {
        curve->SetDiffusionWidth(mGlobalDiffusionGap);
    }
}

void DiffusionCurveRenderer::CurveContainer::SetGlobalBlurStrength(float val)
{
    mGlobalBlurStrength = val;

    for (const auto& curve : mCurves)
    {
        if (BezierPtr bezier = std::dynamic_pointer_cast<Bezier>(curve))
        {
            bezier->SetAllBlurPointsStrength(mGlobalBlurStrength);
        }
    }
}
