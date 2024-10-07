#pragma once

#include "Core/Constants.h"
#include "Structs/Enums.h"
#include "Util/Macros.h"

#include <QVector2D>
#include <QVector4D>
#include <QVector>
#include <memory>

namespace DiffusionCurveRenderer
{
    struct ControlPoint
    {
        QVector2D position;
    };

    struct ColorPoint
    {
        ColorPointType type;
        QVector4D color;
        float position{ 0 };
    };

    struct BlurPoint
    {
        float position{ 0 };
        float strength{ 0 };
    };

    using ControlPointPtr = std::shared_ptr<ControlPoint>;
    using ColorPointPtr = std::shared_ptr<ColorPoint>;
    using BlurPointPtr = std::shared_ptr<BlurPoint>;

    class Curve
    {
      public:
        Curve() = default;
        virtual ~Curve() = default;

        virtual QVector2D PositionAt(float t) const = 0;
        virtual QVector2D TangentAt(float t) const = 0;
        virtual QVector2D NormalAt(float t) const = 0;
        virtual float ParameterAt(const QVector2D& point, int intervals = 1000) const;

        virtual void Update() = 0;

        virtual ControlPointPtr GetControlPoint(int index) = 0;
        virtual QVector2D GetControlPointPosition(int index) const = 0;

        virtual ControlPointPtr AddControlPoint(const QVector2D& position) = 0;
        virtual void RemoveControlPoint(ControlPointPtr point) = 0;
        virtual void RemoveControlPoint(int index) = 0;

        virtual const QVector<ControlPointPtr>& GetControlPoints() const = 0;
        virtual int GetNumberOfControlPoints() const = 0;

        virtual ColorPointPtr FindColorPointAround(const QVector2D& test, float offset, float tolerance) = 0;

        virtual const QVector<QVector2D>& GetControlPointPositions() = 0;

        virtual ColorPointPtr AddColorPoint(ColorPointType type, const QVector4D& color, float position) = 0;
        virtual bool RemoveColorPoint(ColorPointPtr point) = 0;

        virtual BlurPointPtr AddBlurPoint(float position, float strength) = 0;
        virtual bool RemoveBlurPoint(BlurPointPtr point) = 0;

        ControlPointPtr FindControlPointAround(const QVector2D& test, float radius = 8);

        float GetDistanceToPoint(const QVector2D& point, int intervals = 100) const;

        ColorPointPtr TryCreateColorPointAt(const QVector2D& worldPosition) const;

        float CalculateLength(int intervals = 100) const;

      private:
        DEFINE_MEMBER(QVector4D, ContourColor, QVector4D(0, 0, 0, 1));
        DEFINE_MEMBER(float, ContourThickness, DEFAULT_CONTOUR_THICKNESS);
        DEFINE_MEMBER(float, DiffusionWidth, DEFAULT_DIFFUSION_WIDTH);
        DEFINE_MEMBER(float, DiffusionGap, DEFAULT_DIFFUSION_GAP);
    };

    using CurvePtr = std::shared_ptr<Curve>;
}
