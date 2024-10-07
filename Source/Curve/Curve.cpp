#include "Curve.h"

#include <limits>

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Curve::FindControlPointAround(const QVector2D& test, float radius)
{
    ControlPointPtr result = nullptr;

    float minDistance = std::numeric_limits<float>::infinity();

    for (int i = 0; i < GetNumberOfControlPoints(); ++i)
    {
        ControlPointPtr point = GetControlPoint(i);

        const float distance = point->position.distanceToPoint(test);
        if (distance < minDistance)
        {
            minDistance = distance;
            result = point;
        }
    }

    if (radius < minDistance)
    {
        result = nullptr;
    }

    return result;
}

float DiffusionCurveRenderer::Curve::GetDistanceToPoint(const QVector2D& point, int intervals) const
{
    float minDistance = std::numeric_limits<float>::infinity();
    const float dt = 1.0f / intervals;
    float t = 0.0f;

    for (int i = 0; i < intervals; ++i)
    {
        const float distance = PositionAt(t).distanceToPoint(point);

        if (distance < minDistance)
        {
            minDistance = distance;
        }

        t += dt;
    }

    return minDistance;
}

float DiffusionCurveRenderer::Curve::ParameterAt(const QVector2D& point, int intervals) const
{
    float t = 0;
    float dt = 1.0f / intervals;
    float minimumDistance = std::numeric_limits<float>::infinity();
    float parameter = 0;

    for (int i = 0; i <= intervals; i++)
    {
        float distance = PositionAt(t).distanceToPoint(point);
        if (distance < minimumDistance)
        {
            minimumDistance = distance;
            parameter = t;
        }
        t += dt;
    }

    return parameter;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Curve::TryCreateColorPointAt(const QVector2D& worldPosition) const
{
    if (GetNumberOfControlPoints() < 1)
        return nullptr;

    float parameter = ParameterAt(worldPosition);
    QVector3D positionOnCurve = PositionAt(parameter).toVector3D();
    QVector3D tangent = TangentAt(parameter).toVector3D();
    QVector3D direction = (worldPosition.toVector3D() - positionOnCurve).normalized();
    QVector3D cross = QVector3D::crossProduct(tangent, direction);

    ColorPointType type = cross.z() > 0 ? ColorPointType::Left : ColorPointType::Right;

    const auto colorPoint = std::make_shared<ColorPoint>();
    colorPoint->position = parameter;
    colorPoint->type = type;
    colorPoint->color = QVector4D(1, 0, 0, 1);

    return colorPoint;
}

float DiffusionCurveRenderer::Curve::CalculateLength(int intervals) const
{
    float length = 0.0f;
    float t = 0.0f;
    const float dt = 1.0f / static_cast<float>(intervals);

    for (int i = 0; i < intervals; ++i)
    {
        QVector2D v1 = PositionAt(t);
        QVector2D v2 = PositionAt(t + dt);
        length += v1.distanceToPoint(v2);
        t += dt;
    }

    return length;
}
