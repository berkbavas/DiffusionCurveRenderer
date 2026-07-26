#include "Curve.h"

#include <limits>

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Curve::FindControlPointAround(const QVector2D& Test, float Radius)
{
    ControlPointPtr pResult = nullptr;

    float MinDistance = std::numeric_limits<float>::infinity();

    for (int I = 0; I < GetNumberOfControlPoints(); ++I)
    {
        ControlPointPtr pPoint = GetControlPoint(I);

        const auto Distance = pPoint->Position.distanceToPoint(Test);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            pResult = pPoint;
        }
    }

    if (Radius < MinDistance)
    {
        pResult = nullptr;
    }

    return pResult;
}

float DiffusionCurveRenderer::Curve::GetDistanceToPoint(const QVector2D& Point, int Intervals) const
{
    float MinDistance = std::numeric_limits<float>::infinity();
    const float Dt = 1.0f / Intervals;
    float T = 0.0f;

    for (int I = 0; I < Intervals; ++I)
    {
        const float Distance = PositionAt(T).distanceToPoint(Point);

        if (Distance < MinDistance)
        {
            MinDistance = Distance;
        }

        T += Dt;
    }

    return MinDistance;
}

float DiffusionCurveRenderer::Curve::ParameterAt(const QVector2D& Point, int Intervals) const
{
    float T = 0;
    float Dt = 1.0f / Intervals;
    float MinimumDistance = std::numeric_limits<float>::infinity();
    float Parameter = 0;

    for (int I = 0; I <= Intervals; I++)
    {
        float Distance = PositionAt(T).distanceToPoint(Point);
        if (Distance < MinimumDistance)
        {
            MinimumDistance = Distance;
            Parameter = T;
        }
        T += Dt;
    }

    return Parameter;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Curve::TryCreateColorPointAt(const QVector2D& WorldPosition) const
{
    if (GetNumberOfControlPoints() < 1)
    {
        return nullptr;
    }

    const auto Parameter = ParameterAt(WorldPosition);
    const auto PositionOnCurve = PositionAt(Parameter).toVector3D();
    const auto Tangent = TangentAt(Parameter).toVector3D();
    const auto Direction = (WorldPosition.toVector3D() - PositionOnCurve).normalized();
    const auto Cross = QVector3D::crossProduct(Tangent, Direction);

    const auto Type = Cross.z() > 0 ? ColorPointType::Left : ColorPointType::Right;

    const auto pColorPoint = std::make_shared<ColorPoint>();
    pColorPoint->Position = Parameter;
    pColorPoint->Type = Type;
    pColorPoint->Color = QVector4D(1, 0, 0, 1);

    return pColorPoint;
}

float DiffusionCurveRenderer::Curve::CalculateLength(int Intervals) const
{
    float Length = 0.0f;
    float T = 0.0f;
    const float Dt = 1.0f / static_cast<float>(Intervals);

    for (int I = 0; I < Intervals; ++I)
    {
        const auto V1 = PositionAt(T);
        const auto V2 = PositionAt(T + Dt);
        Length += V1.distanceToPoint(V2);
        T += Dt;
    }

    return Length;
}
