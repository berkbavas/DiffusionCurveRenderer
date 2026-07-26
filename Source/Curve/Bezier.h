#pragma once

#include "Curve.h"
#include "Util/Macros.h"

#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QVector>

namespace DiffusionCurveRenderer
{
    class Bezier : public Curve
    {
      public:
        Bezier() = default;
        virtual ~Bezier() = default;

        // Curve interface
        QVector2D PositionAt(float Parameter) const override;
        QVector2D TangentAt(float Parameter) const override;
        QVector2D NormalAt(float Parameter) const override;

        void Update() override;

        ControlPointPtr GetControlPoint(int Index) override;
        QVector2D GetControlPointPosition(int Index) const override;

        ControlPointPtr AddControlPoint(const QVector2D& Position) override;
        void RemoveControlPoint(ControlPointPtr Point) override;
        void RemoveControlPoint(int Index) override;

        const QVector<ControlPointPtr>& GetControlPoints() const override { return mControlPoints; };
        const QVector<QVector2D>& GetControlPointPositions() override;
        int GetNumberOfControlPoints() const override { return mControlPoints.size(); };

        ColorPointPtr AddColorPoint(ColorPointType Type, const QVector4D& Color, float Position) override;
        bool RemoveColorPoint(ColorPointPtr Point) override;

        BlurPointPtr AddBlurPoint(float Position, float Strength) override;
        bool RemoveBlurPoint(BlurPointPtr Point) override;

        ColorPointPtr FindColorPointAround(const QVector2D& Test, float Offset, float Tolerance) override;

        // Bezier
        QVector4D GetLeftColorAt(float Parameter);
        QVector4D GetRightColorAt(float Parameter);

        void SetAllBlurPointsStrength(float Strength);

        const QVector<float>& GetLeftColorPositions();
        const QVector<float>& GetRightColorPositions();

        const QVector<QVector4D>& GetLeftColors();
        const QVector<QVector4D>& GetRightColors();

        const QVector<float>& GetBlurPointPositions();
        const QVector<float>& GetBlurPointStrengths();

        int GetNumberOfLeftColors() const;
        int GetNumberOfRightColors() const;
        int GetNumberOfBlurPoints() const;

        void SortColorPoints();
        void SortBlurPoints();

        int GetOrder() const;
        int GetDegree() const;

        void RemoveAllControlPoints();

        const QVector<ColorPointPtr>& GetColorPoints() const { return mColorPoints; };
        const QVector<BlurPointPtr>& GetBlurPoints() const { return mBlurPoints; };

        QJsonObject ToJsonObject();
        static CurvePtr FromJsonObject(QJsonObject Object);

        // Clone the curve with an optional offset
        std::shared_ptr<Curve> Clone(const QVector2D& Offset = QVector2D(20, 20)) const override;

      private:
        QVector<float> GetCoefficients() const;
        QVector<float> GetDerivativeCoefficients() const;
        float Factorial(int N) const;
        float Choose(int N, int K) const;

        QVector<ControlPointPtr> mControlPoints;
        QVector<ColorPointPtr> mColorPoints;
        QVector<BlurPointPtr> mBlurPoints;

        // Cached
        DEFINE_MEMBER(bool, ControlPointsDirty, true);
        DEFINE_MEMBER(bool, LeftColorsDirty, true);
        DEFINE_MEMBER(bool, LeftColorPositionsDirty, true);
        DEFINE_MEMBER(bool, RightColorsDirty, true);
        DEFINE_MEMBER(bool, RightColorPositionsDirty, true);
        DEFINE_MEMBER(bool, BlurPointPositionsDirty, true);
        DEFINE_MEMBER(bool, BlurPointStrengthsDirty, true);

        QVector<QVector2D> mControlPointPositions;

        QVector<float> mLeftColorPositions;
        QVector<float> mRightColorPositions;
        QVector<QVector4D> mLeftColors;
        QVector<QVector4D> mRightColors;

        QVector<float> mBlurPointPositions;
        QVector<float> mBlurPointStrengths;
    };

    using BezierPtr = std::shared_ptr<Bezier>;
}
