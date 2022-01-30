#ifndef CURVECONTAINER_H
#define CURVECONTAINER_H

#include <Curves/Curve.h>

class CurveContainer : public QObject
{
    Q_OBJECT
public:
    explicit CurveContainer(QObject *parent = nullptr);
    ~CurveContainer();

    void addCurve(Curve *curve);
    void addCurves(const QVector<Curve *> curves);

    QVector<Curve *> &getCurves() const;

    Curve *selectedCurve();
    void setSelectedCurve(Curve *selectedCurve);

    ControlPoint *selectedControlPoint();
    void setSelectedControlPoint(ControlPoint *selectedControlPoint);

    void removeCurve(int index);
    void removeCurve(Curve *curve);

    void deselectAllCurves();

    Curve *selectCurve(const QVector2D &position, float radius = 20.0f);
    Curve *selectCurve(const QPointF &position, float radius = 20.0f);

    ControlPoint *getClosestControlPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 20.0f) const;
    ColorPoint *getClosestColorPointOnSelectedCurve(const QVector2D &nearbyPoint, float radius = 20.0f) const;

    ColorPoint *selectedColorPoint() const;
    void setSelectedColorPoint(ColorPoint *newSelectedColorPoint);

public slots:
    void sortCurves();
    void clear();

signals:
    void selectedCurveChanged(Curve *selectedCurve);
    void selectedControlPointChanged(ControlPoint *selectedControlPoint);
    void selectedColorPointChanged(ColorPoint *selectedColorPoint);

private:
    mutable QVector<Curve *> mCurves;
    Curve *mSelectedCurve;
    ControlPoint *mSelectedControlPoint;
    ColorPoint *mSelectedColorPoint;
};

#endif // CURVECONTAINER_H
