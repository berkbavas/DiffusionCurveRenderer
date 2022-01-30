#include "ColorPoint.h"
#include "Curve.h"

ColorPoint::ColorPoint()
    : mSelected(false)

{}

QVector2D ColorPoint::getPosition2D(float gap) const
{
    if (mParent) {
        QVector2D positionOnCurve = mParent->valueAt(mPosition);
        QVector2D normal = mParent->normalAt(mPosition);

        QVector2D gapVector = (mParent->contourThickness() + gap) * normal;

        if (mType == ColorPoint::Right)
            gapVector = -gapVector;

        return positionOnCurve + gapVector;
    }

    return QVector2D();
}

float ColorPoint::position() const
{
    return mPosition;
}

void ColorPoint::setPosition(float newPosition)
{
    mPosition = newPosition;
}

bool ColorPoint::selected() const
{
    return mSelected;
}

void ColorPoint::setSelected(bool newSelected)
{
    mSelected = newSelected;
}

ColorPoint::Type ColorPoint::type() const
{
    return mType;
}

void ColorPoint::setType(Type newType)
{
    mType = newType;
}

Curve *ColorPoint::parent() const
{
    return mParent;
}

void ColorPoint::setParent(Curve *newParent)
{
    mParent = newParent;
}

const QVector4D &ColorPoint::color() const
{
    return mColor;
}

void ColorPoint::setColor(const QVector4D &newColor)
{
    mColor = newColor;
}
