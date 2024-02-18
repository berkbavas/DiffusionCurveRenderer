#pragma once

#include "BlurPoint.h"
#include "ColorPoint.h"
#include "ControlPoint.h"
#include "Curve.h"

#include <QObject>
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class Bezier : public Curve
    {
    public:
        Bezier();
        virtual ~Bezier();

        void AddControlPoint(ControlPoint* controlPoint);
        void AddColorPoint(ColorPoint* colorPoint);
        void AddBlurPoint(BlurPoint* blurPoint);

        void RemoveControlPoint(int index);
        void RemoveControlPoint(ControlPoint* controlPoint);
        void RemoveLeftColorPoint(int index);
        void RemoveRightColorPoint(int index);
        void RemoveColorPoint(ColorPoint* controlPoint);
        void RemoveBlurPoint(BlurPoint* blurPoint);
        void RemoveBlurPoint(int index);

        QVector<ColorPoint*> GetAllColorPoints() const;

        QVector4D GetLeftColorAt(float t) const;
        QVector4D GetRightColorAt(float t) const;

        void OrderLeftColorPoints();
        void OrderRightColorPoints();
        void OrderBlurPoints();

        QVector<QVector4D> GetLeftColors() const;
        QVector<QVector4D> GetRightColors() const;
        QVector<float> GetBlurPointStrengths() const;

        QVector<QVector2D> GetControlPointPositions() const;
        QVector<float> GetLeftColorPositions() const;
        QVector<float> GetRightColorPositions() const;
        QVector<float> GetBlurPointPositions();

        ControlPoint* GetClosestControlPoint(const QVector2D& nearbyPoint) const;
        ColorPoint* GetClosestColorPoint(const QVector2D& nearbyPoint) const;
        BlurPoint* GetClosestBlurPoint(const QVector2D& nearbyPoint) const;

        int GetOrder() const;
        int GetDegree() const;
        int GetSize() const;

        QVector2D ValueAt(float t) const override;
        QVector2D TangentAt(float t) const override;
        QVector2D NormalAt(float t) const override;
        void Scale(float scaleFactor);

        Bezier* DeepCopy();

        const QList<ControlPoint*>& GetControlPoints() const;
        const QList<ColorPoint*>& GetLeftColorPoints() const;
        const QList<ColorPoint*>& GetRightColorPoints() const;
        const QList<BlurPoint*>& GetBlurPoints() const;

    private:
        QVector<float> GetCoefficients() const;
        QVector<float> GetDerivativeCoefficients() const;
        float Factorial(int n) const;
        float Choose(int n, int k) const;

    public:
        QVector4D mContourColor;
        float mContourThickness;
        float mDiffusionWidth;
        float mDiffusionGap;
        bool mSelected;
        int mDepth;
        bool mVoid;

    private:
        QList<ControlPoint*> mControlPoints;
        QList<ColorPoint*> mLeftColorPoints;
        QList<ColorPoint*> mRightColorPoints;
        QList<BlurPoint*> mBlurPoints;
    };
}
