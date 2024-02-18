#include "BlurPoint.h"
#include "Bezier.h"
#include "Common.h"

DiffusionCurveRenderer::BlurPoint::BlurPoint()
    : mParent(nullptr)
    , mStrength(DEFAULT_BLUR_STRENGTH)
    , mPosition(0.0f)
    , mSelected(false)
{}

void DiffusionCurveRenderer::BlurPoint::SetParent(Bezier* newParent)
{
    mParent = newParent;
}

QVector2D DiffusionCurveRenderer::BlurPoint::GetPosition2D(float gap) const
{
    if (mParent)
    {
        QVector2D vector = (mParent->mContourThickness + gap) * mParent->NormalAt(mPosition);
        return mParent->ValueAt(mPosition) + vector;
    }

    return QVector2D();
}