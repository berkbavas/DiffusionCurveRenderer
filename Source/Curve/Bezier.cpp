#include "Bezier.h"

#include "Util/Chronometer.h"
#include "Util/Logger.h"

#include <QObject>

QVector2D DiffusionCurveRenderer::Bezier::PositionAt(float t) const
{
    const int n = GetDegree();
    QVector2D value = QVector2D(0, 0);

    for (int i = 0; i <= n; i++)
    {
        value += Choose(n, i) * pow(t, i) * pow(1 - t, n - i) * GetControlPointPosition(i);
    }

    return value;
}

QVector2D DiffusionCurveRenderer::Bezier::TangentAt(float t) const
{
    const int order = GetDegree();

    QVector2D tangent = QVector2D(0, 0);

    for (int i = 0; i <= order - 1; i++)
    {
        float coefficient = Choose(order - 1, i) * pow(t, i) * pow(1 - t, order - 1 - i);
        tangent += coefficient * (GetControlPointPosition(i) - GetControlPointPosition(i + 1));
    }

    tangent.normalize();

    return tangent;
}

QVector2D DiffusionCurveRenderer::Bezier::NormalAt(float t) const
{
    QVector2D tangent = TangentAt(t);

    return QVector2D(-tangent.y(), tangent.x());
}

void DiffusionCurveRenderer::Bezier::Update()
{
    mControlPointsDirty = true;
    mLeftColorsDirty = true;
    mLeftColorPositionsDirty = true;
    mRightColorsDirty = true;
    mRightColorPositionsDirty = true;
    mBlurPointPositionsDirty = true;
    mBlurPointStrengthsDirty = true;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Bezier::GetControlPoint(int index)
{
    DCR_ASSERT(0 <= index && index < mControlPoints.size());

    return mControlPoints[index];
}

QVector2D DiffusionCurveRenderer::Bezier::GetControlPointPosition(int index) const
{
    DCR_ASSERT(0 <= index && index < mControlPoints.size());
    return mControlPoints[index]->position;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Bezier::AddControlPoint(const QVector2D& position)
{
    if (mControlPoints.size() >= 32)
    {
        LOG_WARN("Bezier::AddControlPoint: ControlPoint could not be added because the total number of ControlPoints is 32.");
        return nullptr;
    }

    ControlPointPtr point = std::make_shared<ControlPoint>();
    point->position = position;
    mControlPoints << point;
    mControlPointsDirty = true;
    return point;
}

void DiffusionCurveRenderer::Bezier::RemoveControlPoint(ControlPointPtr point)
{
    for (int index = 0; index < mControlPoints.size(); ++index)
    {
        if (point == mControlPoints[index])
        {
            RemoveControlPoint(index);
            return;
        }
    }
}

void DiffusionCurveRenderer::Bezier::RemoveControlPoint(int index)
{
    DCR_ASSERT(0 <= index && index < mControlPoints.size());

    mControlPoints.removeAt(index);
    Update();
}

const QVector<QVector2D>& DiffusionCurveRenderer::Bezier::GetControlPointPositions()
{
    if (mControlPointsDirty)
    {
        mControlPointPositions.clear();

        for (const auto& controlPoint : mControlPoints)
            mControlPointPositions << controlPoint->position;

        mControlPointsDirty = false;
    }

    return mControlPointPositions;
}

void DiffusionCurveRenderer::Bezier::SetAllBlurPointsStrength(float strength)
{
    for (int index = 0; index < mBlurPoints.size(); ++index)
        mBlurPoints[index]->strength = strength;

    mBlurPointStrengthsDirty = true;
}

const QVector<float>& DiffusionCurveRenderer::Bezier::GetLeftColorPositions()
{
    if (mLeftColorPositionsDirty)
    {
        mLeftColorPositions.clear();

        for (const auto colorPoint : mColorPoints)
        {
            if (colorPoint->type == ColorPointType::Left)
                mLeftColorPositions << colorPoint->position;
        }

        mLeftColorPositionsDirty = false;
    }

    return mLeftColorPositions;
}

const QVector<float>& DiffusionCurveRenderer::Bezier::GetRightColorPositions()
{
    if (mRightColorPositionsDirty)
    {
        mRightColorPositions.clear();

        for (const auto colorPoint : mColorPoints)
        {
            if (colorPoint->type == ColorPointType::Right)
                mRightColorPositions << colorPoint->position;
        }

        mRightColorPositionsDirty = false;
    }

    return mRightColorPositions;
}

const QVector<float>& DiffusionCurveRenderer::Bezier::GetBlurPointPositions()
{
    if (mBlurPointPositionsDirty)
    {
        mBlurPointPositions.clear();

        for (const auto& blurPoint : mBlurPoints)
            mBlurPointPositions << blurPoint->position;

        mBlurPointPositionsDirty = false;
    }

    return mBlurPointPositions;
}

const QVector<QVector4D>& DiffusionCurveRenderer::Bezier::GetLeftColors()
{
    if (mLeftColorsDirty)
    {
        mLeftColors.clear();

        for (const auto colorPoint : mColorPoints)
        {
            if (colorPoint->type == ColorPointType::Left)
                mLeftColors << colorPoint->color;
        }

        mLeftColorsDirty = false;
    }

    return mLeftColors;
}

const QVector<QVector4D>& DiffusionCurveRenderer::Bezier::GetRightColors()
{
    if (mRightColorsDirty)
    {
        mRightColors.clear();

        for (const auto colorPoint : mColorPoints)
        {
            if (colorPoint->type == ColorPointType::Right)
                mRightColors << colorPoint->color;
        }

        mRightColorsDirty = false;
    }

    return mRightColors;
}

const QVector<float>& DiffusionCurveRenderer::Bezier::GetBlurPointStrengths()
{
    if (mBlurPointStrengthsDirty)
    {
        mBlurPointStrenghts.clear();

        for (const auto& blurPoint : mBlurPoints)
            mBlurPointStrenghts << blurPoint->strength;

        mBlurPointStrengthsDirty = false;
    }

    return mBlurPointStrenghts;
}

int DiffusionCurveRenderer::Bezier::GetNumberOfLeftColors() const
{
    return mLeftColors.size();
}

int DiffusionCurveRenderer::Bezier::GetNumberOfRightColors() const
{
    return mRightColors.size();
}

int DiffusionCurveRenderer::Bezier::GetNumberOfBlurPoints() const
{
    return mBlurPoints.size();
}

void DiffusionCurveRenderer::Bezier::SortColorPoints()
{
    std::sort(mColorPoints.begin(), mColorPoints.end(), [](ColorPointPtr a, const ColorPointPtr b)
              { return a->position < b->position; });
}

void DiffusionCurveRenderer::Bezier::SortBlurPoints()
{
    std::sort(mBlurPoints.begin(), mBlurPoints.end(), [](BlurPointPtr a, const BlurPointPtr b)
              { return a->position < b->position; });
}

QVector<float> DiffusionCurveRenderer::Bezier::GetCoefficients() const
{
    const int n = GetDegree();

    QVector<float> coefficients;

    for (int i = 0; i <= n; ++i)
    {
        coefficients << Choose(n, i);
    }

    return coefficients;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetDerivativeCoefficients() const
{
    const int n = GetDegree();

    QVector<float> coefficients;

    for (int i = 0; i <= n - 1; ++i)
    {
        coefficients << Choose(n - 1, i);
    }

    return coefficients;
}

float DiffusionCurveRenderer::Bezier::Factorial(int n) const
{
    double result = 1.0;

    for (int i = 1; i <= n; ++i)
    {
        result *= i;
    }

    return result;
}

float DiffusionCurveRenderer::Bezier::Choose(int n, int k) const
{
    return Factorial(n) / (Factorial(k) * Factorial(n - k));
}

int DiffusionCurveRenderer::Bezier::GetOrder() const
{
    return mControlPoints.size();
}

int DiffusionCurveRenderer::Bezier::GetDegree() const
{
    return mControlPoints.size() - 1;
}

void DiffusionCurveRenderer::Bezier::RemoveAllControlPoints()
{
    mControlPoints.clear();
    mControlPointPositions.clear();
    Update();
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Bezier::AddColorPoint(ColorPointType type, const QVector4D& color, float position)
{
    if (mColorPoints.size() >= 16)
    {
        LOG_WARN("Bezier::AddColorPoint: ColorPoint could not be added because the total number of ColorPoints is 16.");
        return nullptr;
    }

    ColorPointPtr point = std::make_shared<ColorPoint>(type, color, position);
    mColorPoints << point;
    SortColorPoints();
    Update();

    return point;
}

bool DiffusionCurveRenderer::Bezier::RemoveColorPoint(ColorPointPtr point)
{
    int index = -1;
    for (int i = 0; i < mColorPoints.size(); ++i)
    {
        if (mColorPoints[i] == point)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        mColorPoints.remove(index);
        SortColorPoints();
        Update();

        return true;
    }

    return false;
}

DiffusionCurveRenderer::BlurPointPtr DiffusionCurveRenderer::Bezier::AddBlurPoint(float position, float strength)
{
    if (mBlurPoints.size() >= 16)
    {
        LOG_WARN("Bezier::AddBlurPoint: BlurPoint could not be added because the total number of BlurPoints is 16.");
        return nullptr;
    }

    BlurPointPtr point = std::make_shared<BlurPoint>(position, strength);
    mBlurPoints << point;
    SortBlurPoints();
    Update();

    return point;
}

bool DiffusionCurveRenderer::Bezier::RemoveBlurPoint(BlurPointPtr point)
{
    int index = -1;
    for (int i = 0; i < mBlurPoints.size(); ++i)
    {
        if (mBlurPoints[i] == point)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        mBlurPoints.remove(index);
        SortBlurPoints();
        Update();

        return true;
    }

    return false;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Bezier::FindColorPointAround(const QVector2D& test, float offset, float tolerance)
{
    MEASURE_CALL_TIME(BEZIER_FIND_COLOR_POINT_AROUND);

    ColorPointPtr result = nullptr;

    float minDistance = std::numeric_limits<float>::infinity();

    for (const auto& colorPoint : mColorPoints)
    {
        offset = colorPoint->type == ColorPointType::Left ? -offset : offset;

        QVector2D positionOnCurve = PositionAt(colorPoint->position);
        QVector2D translatedPosition = positionOnCurve + NormalAt(colorPoint->position) * offset;

        float distance = translatedPosition.distanceToPoint(test);

        if (distance < minDistance)
        {
            minDistance = distance;
            result = colorPoint;
        }
    }

    if (tolerance < minDistance)
        result = nullptr;

    return result;
}

QVector4D DiffusionCurveRenderer::Bezier::GetLeftColorAt(float t)
{
    const auto& colors = GetLeftColors();
    const auto& positions = GetLeftColorPositions();

    DCR_ASSERT(colors.size() == positions.size());

    if (colors.size() == 0 || colors.size() == 1)
    {
        return QVector4D(0, 0, 0, 0);
    }

    for (int i = 1; i < positions.size(); ++i)
    {
        if (positions[i - 1] <= t && t <= positions[i])
        {
            float s = (t - positions[i - 1]) / (positions[i] - positions[i - 1]);
            return colors[i - 1] + s * (colors[i] - colors[i - 1]);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

QVector4D DiffusionCurveRenderer::Bezier::GetRightColorAt(float t)
{
    const auto& colors = GetRightColors();
    const auto& positions = GetRightColorPositions();

    DCR_ASSERT(colors.size() == positions.size());

    if (colors.size() == 0 || colors.size() == 1)
    {
        return QVector4D(0, 0, 0, 0);
    }

    for (int i = 1; i < positions.size(); ++i)
    {
        if (positions[i - 1] <= t && t <= positions[i])
        {
            float s = (t - positions[i - 1]) / (positions[i] - positions[i - 1]);
            return colors[i - 1] + s * (colors[i] - colors[i - 1]);
        }
    }

    return QVector4D(0, 0, 0, 0);
}
