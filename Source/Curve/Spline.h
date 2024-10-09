#pragma once

#include "Curve/Bezier.h"

#include <Eigen/Dense>
#include <QVector>

namespace DiffusionCurveRenderer
{
    class Spline : public Curve
    {
      public:
        Spline() = default;

        // Curve Interface
        QVector2D PositionAt(float t) const override;
        QVector2D TangentAt(float t) const override;
        QVector2D NormalAt(float t) const override;

        void Update() override;

        ControlPointPtr GetControlPoint(int index) override;
        QVector2D GetControlPointPosition(int index) const override;

        ControlPointPtr AddControlPoint(const QVector2D& position) override;
        void RemoveControlPoint(ControlPointPtr point) override;
        void RemoveControlPoint(int index) override;

        const QVector<ControlPointPtr>& GetControlPoints() const override { return mControlPoints; }
        const QVector<QVector2D>& GetControlPointPositions() override;
        int GetNumberOfControlPoints() const override { return mControlPoints.size(); }

        ColorPointPtr AddColorPoint(ColorPointType type, const QVector4D& color, float position) override;
        bool RemoveColorPoint(ColorPointPtr point) override;

        BlurPointPtr AddBlurPoint(float position, float strength) override;
        bool RemoveBlurPoint(BlurPointPtr point) override;

        ColorPointPtr FindColorPointAround(const QVector2D& test, float offset, float tolerance) override;

        // Spline
        const QVector<BezierPtr>& GetBezierPatches() const { return mBezierPatches; };

        BezierPtr GetBezierPatchAt(float t) const;
        int GetBezierPatchIndexAt(float t) const;
        float TransformToPatch(float t) const;
        float TransformToSpline(int patchIndex, float t) const;

        QJsonObject ToJsonObject();
        static CurvePtr FromJsonObject(QJsonObject object);

      private:
        Eigen::MatrixXf CreateCoefficientMatrix();
        QVector<QVector2D> GetSplineControlPoints();

        void SaveColorPoints();
        void RestoreColorPoints();

      private:
        QVector<ControlPointPtr> mControlPoints;
        QVector<BezierPtr> mBezierPatches;

        QVector<QVector2D> mControlPointPositions;
        bool mControlPointPositionsDirty{ true };

        bool mIsPointAddedOrRemoved{ false };

        QVector<ColorPointPtr> mColorsBeforeUpdate;
    };

    using SplinePtr = std::shared_ptr<Spline>;
}