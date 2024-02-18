#include "ColorPoint.h"
#include "Bezier.h"

DiffusionCurveRenderer::ColorPoint::ColorPoint()
    : mColor(1, 1, 1, 1)
    , mPosition(0)
    , mSelected(false)
    , mParent(nullptr)
{}

QVector2D DiffusionCurveRenderer::ColorPoint::GetPosition2D(float gap) const
{
    if (mParent)
    {
        QVector2D vector = (mParent->mContourThickness + gap) * mParent->NormalAt(mPosition);

        if (mDirection == DiffusionCurveRenderer::ColorPoint::Direction::Right)
            vector = -vector;

        return mParent->ValueAt(mPosition) + vector;
    }

    return QVector2D();
}

void DiffusionCurveRenderer::ColorPoint::SetParent(Bezier* newParent)
{
    mParent = newParent;
}