#include "ControlPoint.h"

DiffusionCurveRenderer::ControlPoint::ControlPoint()
    : mSelected(false)
{}

DiffusionCurveRenderer::ControlPoint* DiffusionCurveRenderer::ControlPoint::DeepCopy() const
{
    ControlPoint* point = new ControlPoint;
    point->mPosition = mPosition;
    point->mSelected = mSelected;
    return point;
}