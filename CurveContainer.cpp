#include "CurveContainer.h"

CurveContainer::CurveContainer(QObject *parent)
    : QObject(parent)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
{}

CurveContainer::~CurveContainer()
{
    for (auto &curve : mCurves) {
        if (curve)
            curve->deleteLater();

        curve = nullptr;
    }
}

void CurveContainer::addCurve(Curve *curve)
{
    mCurves << curve;
    sortCurves();
}

void CurveContainer::addCurves(const QVector<Curve *> curves)
{
    mCurves << curves;
    sortCurves();
}

QVector<Curve *> &CurveContainer::getCurves() const
{
    return mCurves;
}

ControlPoint *CurveContainer::getClosestControlPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    ControlPoint *controlPoint = const_cast<ControlPoint *>(mSelectedCurve->getClosestControlPoint(nearbyPoint));

    if (controlPoint) {
        if (controlPoint->position().distanceToPoint(nearbyPoint) > radius)
            controlPoint = nullptr;
    }

    return controlPoint;
}

ColorPoint *CurveContainer::getClosestColorPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    ColorPoint *colorPoint = const_cast<ColorPoint *>(mSelectedCurve->getClosestColorPoint(nearbyPoint));

    if (colorPoint) {
        if (colorPoint->getPosition2D().distanceToPoint(nearbyPoint) > radius)
            colorPoint = nullptr;
    }

    return colorPoint;
}

Curve *CurveContainer::selectedCurve()
{
    return mSelectedCurve;
}

ControlPoint *CurveContainer::selectedControlPoint()
{
    return mSelectedControlPoint;
}

void CurveContainer::removeCurve(int index)
{
    if (0 <= index && index < mCurves.size()) {
        Curve *curve = mCurves[index];
        mCurves.removeAt(index);
        curve->deleteLater();
        sortCurves();
    }
}

void CurveContainer::removeCurve(Curve *curve)
{
    for (int i = 0; i < mCurves.size(); ++i) {
        if (mCurves[i] == curve) {
            removeCurve(i);
            sortCurves();
            return;
        }
    }
}

Curve *CurveContainer::selectCurve(const QVector2D &position, float radius)
{
    float minDistance = std::numeric_limits<float>::infinity();

    Curve *selectedCurve = nullptr;

    for (int i = 0; i < mCurves.size(); ++i) {
        float distance = mCurves[i]->distanceToPoint(position);
        if (distance < minDistance) {
            minDistance = distance;
            selectedCurve = mCurves[i];
        }
    }

    if (minDistance < radius)
        return selectedCurve;

    return nullptr;
}

Curve *CurveContainer::selectCurve(const QPointF &position, float radius)
{
    return selectCurve(QVector2D(position.x(), position.y()), radius);
}

void CurveContainer::deselectAllCurves()
{
    for (int i = 0; i < mCurves.size(); ++i)
        mCurves[i]->setSelected(false);

    mSelectedCurve = nullptr;
}

void CurveContainer::setSelectedCurve(Curve *selectedCurve)
{
    if (mSelectedCurve == selectedCurve)
        return;

    if (mSelectedCurve)
        mSelectedCurve->setSelected(false);

    if (selectedCurve)
        selectedCurve->setSelected(true);

    mSelectedCurve = selectedCurve;
    emit selectedCurveChanged(selectedCurve);
}

void CurveContainer::setSelectedControlPoint(ControlPoint *selectedControlPoint)
{
    if (mSelectedControlPoint == selectedControlPoint)
        return;

    if (mSelectedControlPoint)
        mSelectedControlPoint->setSelected(false);

    if (selectedControlPoint)
        selectedControlPoint->setSelected(true);

    mSelectedControlPoint = selectedControlPoint;
    emit selectedControlPointChanged(selectedControlPoint);
}

void CurveContainer::sortCurves()
{
    if (mCurves.size() == 0 || mCurves.size() == 1)
        return;

    QVector<Curve *> sortedCurves;

    sortedCurves << mCurves[0];

    for (int i = 1; i < mCurves.size(); i++) {
        Curve *curve = mCurves[i];
        if (sortedCurves.last()->z() <= curve->z())
            sortedCurves << curve;
        else
            for (int j = 0; j < sortedCurves.size(); j++)
                if (sortedCurves[j]->z() > curve->z()) {
                    sortedCurves.insert(j, curve);
                    break;
                }
    }

    mCurves = sortedCurves;
}

void CurveContainer::clear()
{
    for (auto &curve : mCurves) {
        if (curve)
            curve->deleteLater();

        curve = nullptr;
    }

    mCurves.clear();
}

ColorPoint *CurveContainer::selectedColorPoint() const
{
    return mSelectedColorPoint;
}

void CurveContainer::setSelectedColorPoint(ColorPoint *newSelectedColorPoint)
{
    if (mSelectedColorPoint == newSelectedColorPoint)
        return;

    if (mSelectedColorPoint)
        mSelectedColorPoint->setSelected(false);

    if (newSelectedColorPoint)
        newSelectedColorPoint->setSelected(true);

    mSelectedColorPoint = newSelectedColorPoint;
    emit selectedColorPointChanged(newSelectedColorPoint);
}
