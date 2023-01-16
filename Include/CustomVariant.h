#pragma once

#include <QVariant>
#include <QVector2D>
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class CustomVariant : public QVariant
    {
    public:
        using QVariant::QVariant;
        CustomVariant(QVector4D);
        CustomVariant(QVector2D);

        QVector4D toVector4D();
        QVector2D toVector2D();

    private:
        QVector4D mVector4D;
        QVector2D mVector2D;
    };
}
