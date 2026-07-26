#pragma once

#include "Core/Constants.h"
#include "Structs/Enums.h"
#include "Util/Macros.h"

#include <memory>

#include <QVector2D>
#include <QVector4D>
#include <QVector>

namespace DiffusionCurveRenderer
{
    struct ControlPoint
    {
        QVector2D Position;
    };

    struct ColorPoint
    {
        ColorPointType Type;
        QVector4D Color;
        float Position{ 0 };
    };

    struct BlurPoint
    {
        float Position{ 0 };
        float Strength{ 0 };
    };

    using ControlPointPtr = std::shared_ptr<ControlPoint>;
    using ColorPointPtr = std::shared_ptr<ColorPoint>;
    using BlurPointPtr = std::shared_ptr<BlurPoint>;

    class Curve
    {
      public:
        Curve() = default;
        virtual ~Curve() = default;

        virtual QVector2D PositionAt(float Parameter) const = 0;
        virtual QVector2D TangentAt(float Parameter) const = 0;
        virtual QVector2D NormalAt(float Parameter) const = 0;
        virtual float ParameterAt(const QVector2D& Point, int Intervals = 1000) const;
        virtual void Update() = 0;
        virtual ControlPointPtr GetControlPoint(int Index) = 0;
        virtual QVector2D GetControlPointPosition(int Index) const = 0;
        virtual ControlPointPtr AddControlPoint(const QVector2D& Position) = 0;
        virtual void RemoveControlPoint(ControlPointPtr Point) = 0;
        virtual void RemoveControlPoint(int Index) = 0;
        virtual const QVector<ControlPointPtr>& GetControlPoints() const = 0;
        virtual int GetNumberOfControlPoints() const = 0;
        virtual ColorPointPtr FindColorPointAround(const QVector2D& Test, float Offset, float Tolerance) = 0;
        virtual const QVector<QVector2D>& GetControlPointPositions() = 0;
        virtual ColorPointPtr AddColorPoint(ColorPointType Type, const QVector4D& Color, float Position) = 0;
        virtual bool RemoveColorPoint(ColorPointPtr Point) = 0;
        virtual BlurPointPtr AddBlurPoint(float Position, float Strength) = 0;
        virtual bool RemoveBlurPoint(BlurPointPtr Point) = 0;
        virtual std::shared_ptr<Curve> Clone(const QVector2D& Offset = QVector2D(20, 20)) const = 0;

        ControlPointPtr FindControlPointAround(const QVector2D& Test, float Radius = 8);
        float GetDistanceToPoint(const QVector2D& Point, int Intervals = 100) const;
        ColorPointPtr TryCreateColorPointAt(const QVector2D& WorldPosition) const;
        float CalculateLength(int Intervals = 100) const;

      private:
        DEFINE_MEMBER(QVector4D, ContourColor, QVector4D(0, 0, 0, 1));
        DEFINE_MEMBER(float, ContourThickness, DEFAULT_CONTOUR_THICKNESS);
        DEFINE_MEMBER(float, DiffusionWidth, DEFAULT_DIFFUSION_WIDTH);
        DEFINE_MEMBER(float, DiffusionGap, DEFAULT_DIFFUSION_GAP);
    };

    using CurvePtr = std::shared_ptr<Curve>;
}
