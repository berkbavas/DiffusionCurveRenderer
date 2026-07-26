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
        QVector2D PositionAt(float Parameter) const override;
        QVector2D TangentAt(float Parameter) const override;
        QVector2D NormalAt(float tParameter) const override;
        void Update() override;
        ControlPointPtr GetControlPoint(int Index) override;
        QVector2D GetControlPointPosition(int Index) const override;
        ControlPointPtr AddControlPoint(const QVector2D& Position) override;
        void RemoveControlPoint(ControlPointPtr pPoint) override;
        void RemoveControlPoint(int Index) override;
        const QVector<ControlPointPtr>& GetControlPoints() const override { return mControlPoints; }
        const QVector<QVector2D>& GetControlPointPositions() override;
        int GetNumberOfControlPoints() const override { return mControlPoints.size(); }
        ColorPointPtr AddColorPoint(ColorPointType Type, const QVector4D& Color, float Position) override;
        bool RemoveColorPoint(ColorPointPtr pPoint) override;
        BlurPointPtr AddBlurPoint(float Position, float Strength) override;
        bool RemoveBlurPoint(BlurPointPtr pPoint) override;
        ColorPointPtr FindColorPointAround(const QVector2D& Test, float Offset, float Tolerance) override;
        std::shared_ptr<Curve> Clone(const QVector2D& Offset = QVector2D(20, 20)) const override;

        // Spline
        const QVector<BezierPtr>& GetBezierPatches() const { return mBezierPatches; };

        BezierPtr GetBezierPatchAt(float Parameter) const;
        int GetBezierPatchIndexAt(float Parameter) const;
        float TransformToPatch(float Parameter) const;
        float TransformToSpline(int PatchIndex, float Parameter) const;

        QJsonObject ToJsonObject();
        static CurvePtr FromJsonObject(QJsonObject Object);

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