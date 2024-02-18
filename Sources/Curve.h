#pragma once

#include <QVector2D>
#include <QVector>

namespace DiffusionCurveRenderer
{
    class Curve
    {
    public:
        Curve();
        virtual ~Curve();

        virtual QVector2D ValueAt(float t) const = 0;
        virtual QVector2D TangentAt(float t) const = 0;
        virtual QVector2D NormalAt(float t) const = 0;

        virtual float ParameterAt(const QVector2D& point, int intervals = 1000) const;
        virtual float GetLength(int intervals = 100) const;
        virtual QRectF GetBoundingBox(int intervals = 100) const;
        virtual float GetDistanceToPoint(const QVector2D& point, int intervals = 100) const;

    private:
        static QVector2D FindMeanCenter(const QVector<QVector2D>& points);
        static QVector<QVector2D> Translate(const QVector<QVector2D>& points, const QVector2D& translation);
        static void FindLineOfBestFit(const QVector<QVector2D>& points, QVector2D& startingPoint, QVector2D& direction, int segments = 1000);
        static float GetAverageDistanceToLine(const QVector<QVector2D>& points, const QVector2D& startingPoint, const QVector2D& direction);
        static float PerpendicularAt(const QVector2D& startingPoint, const QVector2D& direction, const QVector2D& subject);
        static float GetDistanceToLineSegment(const QVector2D& startingPoint, const QVector2D& endPoint, const QVector2D& subject, int intervals = 100);
    };
}
