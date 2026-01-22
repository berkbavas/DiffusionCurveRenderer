#pragma once

#include "Curve.h"
#include "Util/Macros.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QVector>
#include <memory>

namespace DiffusionCurveRenderer
{

    class Bezier : public Curve
    {
      public:
        Bezier() = default;
        virtual ~Bezier() = default;

        // Curve interface
        QVector2D PositionAt(float t) const override;
        QVector2D TangentAt(float t) const override;
        QVector2D NormalAt(float t) const override;

        void Update() override;

        ControlPointPtr GetControlPoint(int index) override;
        QVector2D GetControlPointPosition(int index) const override;

        ControlPointPtr AddControlPoint(const QVector2D& position) override;
        void RemoveControlPoint(ControlPointPtr point) override;
        void RemoveControlPoint(int index) override;

        const QVector<ControlPointPtr>& GetControlPoints() const override { return mControlPoints; };
        const QVector<QVector2D>& GetControlPointPositions() override;
        int GetNumberOfControlPoints() const override { return mControlPoints.size(); };

        ColorPointPtr AddColorPoint(ColorPointType type, const QVector4D& color, float position) override;
        bool RemoveColorPoint(ColorPointPtr point) override;

        BlurPointPtr AddBlurPoint(float position, float strength) override;
        bool RemoveBlurPoint(BlurPointPtr point) override;

        ColorPointPtr FindColorPointAround(const QVector2D& test, float offset, float tolerance) override;

        // Bezier
        QVector4D GetLeftColorAt(float t);
        QVector4D GetRightColorAt(float t);

        void SetAllBlurPointsStrength(float strength);

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
        static CurvePtr FromJsonObject(QJsonObject object);
        
        // Clone the curve with an optional offset
        std::shared_ptr<Curve> Clone(const QVector2D& offset = QVector2D(20, 20)) const override;

      private:
        QVector<float> GetCoefficients() const;
        QVector<float> GetDerivativeCoefficients() const;
        float Factorial(int n) const;
        float Choose(int n, int k) const;

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
        QVector<float> mBlurPointStrenghts;
    };

    using BezierPtr = std::shared_ptr<Bezier>;
}
