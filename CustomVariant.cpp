#include "CustomVariant.h"

CustomVariant::CustomVariant(QVector4D vector)
    : mVector4D(vector)
{}

CustomVariant::CustomVariant(QVector2D vector)
    : mVector2D(vector)
{}

QVector4D CustomVariant::toVector4D()
{
    return mVector4D;
}

QVector2D CustomVariant::toVector2D()
{
    return mVector2D;
}
