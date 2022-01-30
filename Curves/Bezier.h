#ifndef BEZIER_H
#define BEZIER_H

#include "ControlPoint.h"
#include "Curve.h"

#include <QRectF>
#include <QVector2D>
#include <QVector4D>
#include <QVector>

class Bezier : public Curve
{
public:
    Bezier();
    ~Bezier();

    QVector2D valueAt(float t) const override;
    QVector2D tangentAt(float t) const override;
    QVector2D normalAt(float t) const override;

private:
    QVector<float> getCoefficients() const;
    QVector<float> getDerivativeCoefficients() const;
    float factorial(int n) const;
    float choose(int n, int k) const;
};

#endif // BEZIER_H
