#include "ControlPoint.h"

ControlPoint::ControlPoint(QVector2D position)
    : mPosition(position)
    , mSelected(false)
{}

const QVector2D &ControlPoint::position() const
{
    return mPosition;
}

void ControlPoint::setPosition(const QVector2D &newPosition)
{
    mPosition = newPosition;
}

bool ControlPoint::selected() const
{
    return mSelected;
}

void ControlPoint::setSelected(bool newSelected)
{
    mSelected = newSelected;
}

int ControlPoint::index() const
{
    return mIndex;
}

void ControlPoint::setIndex(int newIndex)
{
    mIndex = newIndex;
}
