#include "Bezier.h"

Bezier::Bezier() {}

Bezier::~Bezier() {}

QVector2D Bezier::valueAt(float t) const
{
    QVector2D value = QVector2D(0, 0);
    int n = getDegree();

    for (int i = 0; i <= n; i++)
        value += choose(n, i) * pow(t, i) * pow(1 - t, n - i) * mControlPoints[i]->position();

    return value;
}

QVector2D Bezier::tangentAt(float t) const
{
    QVector2D tangent = QVector2D(0, 0);
    int order = getDegree();

    for (int i = 0; i <= order - 1; i++) {
        float coefficient = choose(order - 1, i) * pow(t, i) * pow(1 - t, order - 1 - i);
        tangent += coefficient * (mControlPoints.at(i)->position() - mControlPoints.at(i + 1)->position());
    }

    tangent.normalize();

    return tangent;
}

QVector2D Bezier::normalAt(float t) const
{
    QVector2D tangent = tangentAt(t);

    return QVector2D(-tangent.y(), tangent.x());
}

QVector<float> Bezier::getCoefficients() const
{
    QVector<float> coefficients;
    int n = getDegree();

    for (int i = 0; i <= n; ++i)
        coefficients << choose(n, i);

    return coefficients;
}

QVector<float> Bezier::getDerivativeCoefficients() const
{
    QVector<float> coefficients;
    int n = getDegree();

    for (int i = 0; i <= n - 1; ++i)
        coefficients << choose(n - 1, i);

    return coefficients;
}

float Bezier::factorial(int n) const
{
    double result = 1.0;

    for (int i = 1; i <= n; ++i)
        result *= i;

    return result;
}

float Bezier::choose(int n, int k) const
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}
