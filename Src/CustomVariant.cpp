#include "CustomVariant.h"

DiffusionCurveRenderer::CustomVariant::CustomVariant(QVector4D vector)
    : mVector4D(vector)
{}

DiffusionCurveRenderer::CustomVariant::CustomVariant(QVector2D vector)
    : mVector2D(vector)
{}

QVector4D DiffusionCurveRenderer::CustomVariant::toVector4D()
{
    return mVector4D;
}

QVector2D DiffusionCurveRenderer::CustomVariant::toVector2D()
{
    return mVector2D;
}