#include "Bezier.h"

#include "Util/Chronometer.h"
#include "Util/Logger.h"

#include <QObject>

QVector2D DiffusionCurveRenderer::Bezier::PositionAt(float Parameter) const
{
    const int N = GetDegree();
    QVector2D Value = QVector2D(0, 0);

    for (int i = 0; i <= N; i++)
    {
        Value += Choose(N, i) * pow(Parameter, i) * pow(1 - Parameter, N - i) * GetControlPointPosition(i);
    }

    return Value;
}

QVector2D DiffusionCurveRenderer::Bezier::TangentAt(float Parameter) const
{
    const int Order = GetDegree();

    QVector2D Tangent = QVector2D(0, 0);

    for (int i = 0; i <= Order - 1; i++)
    {
        const auto Coefficient = Choose(Order - 1, i) * pow(Parameter, i) * pow(1 - Parameter, Order - 1 - i);
        Tangent += Coefficient * (GetControlPointPosition(i) - GetControlPointPosition(i + 1));
    }

    Tangent.normalize();

    return Tangent;
}

QVector2D DiffusionCurveRenderer::Bezier::NormalAt(float Parameter) const
{
    QVector2D Tangent = TangentAt(Parameter);

    return QVector2D(-Tangent.y(), Tangent.x());
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

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Bezier::GetControlPoint(int Index)
{
    DCR_ASSERT(0 <= Index && Index < mControlPoints.size());

    return mControlPoints[Index];
}

QVector2D DiffusionCurveRenderer::Bezier::GetControlPointPosition(int Index) const
{
    DCR_ASSERT(0 <= Index && Index < mControlPoints.size());

    return mControlPoints[Index]->Position;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Bezier::AddControlPoint(const QVector2D& Position)
{
    if (mControlPoints.size() >= 32)
    {
        LOG_WARN("Bezier::AddControlPoint: ControlPoint could not be added because the total number of ControlPoints is 32.");
        return nullptr;
    }

    ControlPointPtr pPoint = std::make_shared<ControlPoint>();
    pPoint->Position = Position;
    mControlPoints << pPoint;
    mControlPointsDirty = true;
    return pPoint;
}

void DiffusionCurveRenderer::Bezier::RemoveControlPoint(ControlPointPtr pPoint)
{
    for (int Index = 0; Index < mControlPoints.size(); ++Index)
    {
        if (pPoint == mControlPoints[Index])
        {
            RemoveControlPoint(Index);
            return;
        }
    }
}

void DiffusionCurveRenderer::Bezier::RemoveControlPoint(int Index)
{
    DCR_ASSERT(0 <= Index && Index < mControlPoints.size());

    mControlPoints.removeAt(Index);
    Update();
}

const QVector<QVector2D>& DiffusionCurveRenderer::Bezier::GetControlPointPositions()
{
    if (mControlPointsDirty)
    {
        mControlPointPositions.clear();

        for (const auto& pControlPoint : mControlPoints)
        {
            mControlPointPositions << pControlPoint->Position;
        }

        mControlPointsDirty = false;
    }

    return mControlPointPositions;
}

void DiffusionCurveRenderer::Bezier::SetAllBlurPointsStrength(float Strength)
{
    for (int Index = 0; Index < mBlurPoints.size(); ++Index)
    {
        mBlurPoints[Index]->Strength = Strength;
    }

    mBlurPointStrengthsDirty = true;
}

const QVector<float>& DiffusionCurveRenderer::Bezier::GetLeftColorPositions()
{
    if (mLeftColorPositionsDirty)
    {
        mLeftColorPositions.clear();

        for (const auto pColorPoint : mColorPoints)
        {
            if (pColorPoint->Type == ColorPointType::Left)
            {
                mLeftColorPositions << pColorPoint->Position;
            }
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

        for (const auto pColorPoint : mColorPoints)
        {
            if (pColorPoint->Type == ColorPointType::Right)
            {
                mRightColorPositions << pColorPoint->Position;
            }
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

        for (const auto& pBlurPoint : mBlurPoints)
        {
            mBlurPointPositions << pBlurPoint->Position;
        }

        mBlurPointPositionsDirty = false;
    }

    return mBlurPointPositions;
}

const QVector<QVector4D>& DiffusionCurveRenderer::Bezier::GetLeftColors()
{
    if (mLeftColorsDirty)
    {
        mLeftColors.clear();

        for (const auto& pColorPoint : mColorPoints)
        {
            if (pColorPoint->Type == ColorPointType::Left)
            {
                mLeftColors << pColorPoint->Color;
            }
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

        for (const auto& pColorPoint : mColorPoints)
        {
            if (pColorPoint->Type == ColorPointType::Right)
            {
                mRightColors << pColorPoint->Color;
            }
        }

        mRightColorsDirty = false;
    }

    return mRightColors;
}

const QVector<float>& DiffusionCurveRenderer::Bezier::GetBlurPointStrengths()
{
    if (mBlurPointStrengthsDirty)
    {
        mBlurPointStrengths.clear();

        for (const auto& pBlurPoint : mBlurPoints)
        {
            mBlurPointStrengths << pBlurPoint->Strength;
        }

        mBlurPointStrengthsDirty = false;
    }

    return mBlurPointStrengths;
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
    std::sort(mColorPoints.begin(), mColorPoints.end(), [](const ColorPointPtr& A, const ColorPointPtr& B) { return A->Position < B->Position; });
}

void DiffusionCurveRenderer::Bezier::SortBlurPoints()
{
    std::sort(mBlurPoints.begin(), mBlurPoints.end(), [](const BlurPointPtr& A, const BlurPointPtr& B) { return A->Position < B->Position; });
}

QVector<float> DiffusionCurveRenderer::Bezier::GetCoefficients() const
{
    const auto N = GetDegree();

    QVector<float> Coefficients;

    for (int I = 0; I <= N; ++I)
    {
        Coefficients << Choose(N, I);
    }

    return Coefficients;
}

QVector<float> DiffusionCurveRenderer::Bezier::GetDerivativeCoefficients() const
{
    const auto N = GetDegree();

    QVector<float> DerivativeCoefficients;

    for (int I = 0; I <= N - 1; ++I)
    {
        DerivativeCoefficients << Choose(N - 1, I);
    }

    return DerivativeCoefficients;
}

float DiffusionCurveRenderer::Bezier::Factorial(int N) const
{
    double Result = 1.0;

    for (int I = 1; I <= N; ++I)
    {
        Result *= float(I);
    }

    return Result;
}

float DiffusionCurveRenderer::Bezier::Choose(int N, int K) const
{
    return Factorial(N) / (Factorial(K) * Factorial(N - K));
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

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Bezier::AddColorPoint(ColorPointType Type, const QVector4D& Color, float Position)
{
    if (mColorPoints.size() >= 16)
    {
        LOG_WARN("Bezier::AddColorPoint: ColorPoint could not be added because the total number of ColorPoints is 16.");
        return nullptr;
    }

    ColorPointPtr pPoint = std::make_shared<ColorPoint>(Type, Color, Position);
    mColorPoints << pPoint;
    SortColorPoints();
    Update();

    return pPoint;
}

bool DiffusionCurveRenderer::Bezier::RemoveColorPoint(ColorPointPtr pPoint)
{
    int Index = -1;
    for (int I = 0; I < mColorPoints.size(); ++I)
    {
        if (mColorPoints[I] == pPoint)
        {
            Index = I;
            break;
        }
    }

    if (Index != -1)
    {
        mColorPoints.remove(Index);
        SortColorPoints();
        Update();

        return true;
    }

    return false;
}

DiffusionCurveRenderer::BlurPointPtr DiffusionCurveRenderer::Bezier::AddBlurPoint(float Position, float Strength)
{
    if (mBlurPoints.size() >= 16)
    {
        LOG_WARN("Bezier::AddBlurPoint: BlurPoint could not be added because the total number of BlurPoints is 16.");
        return nullptr;
    }

    BlurPointPtr pPoint = std::make_shared<BlurPoint>(Position, Strength);
    mBlurPoints << pPoint;
    SortBlurPoints();
    Update();

    return pPoint;
}

bool DiffusionCurveRenderer::Bezier::RemoveBlurPoint(BlurPointPtr pPoint)
{
    int Index = -1;
    for (int I = 0; I < mBlurPoints.size(); ++I)
    {
        if (mBlurPoints[I] == pPoint)
        {
            Index = I;
            break;
        }
    }

    if (Index != -1)
    {
        mBlurPoints.remove(Index);
        SortBlurPoints();
        Update();

        return true;
    }

    return false;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Bezier::FindColorPointAround(const QVector2D& Test, float Offset, float Tolerance)
{
    MEASURE_CALL_TIME(BEZIER_FIND_COLOR_POINT_AROUND);

    ColorPointPtr pResult = nullptr;

    float MinDistance = std::numeric_limits<float>::infinity();

    for (const auto& pColorPoint : mColorPoints)
    {
        Offset = pColorPoint->Type == ColorPointType::Left ? -Offset : Offset;

        const auto PositionOnCurve = PositionAt(pColorPoint->Position);
        const auto TranslatedPosition = PositionOnCurve + NormalAt(pColorPoint->Position) * Offset;
        const auto Distance = TranslatedPosition.distanceToPoint(Test);

        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            pResult = pColorPoint;
        }
    }

    if (Tolerance < MinDistance)
    {
        pResult = nullptr;
    }

    return pResult;
}

QVector4D DiffusionCurveRenderer::Bezier::GetLeftColorAt(float Parameter)
{
    const auto& Colors = GetLeftColors();
    const auto& Positions = GetLeftColorPositions();

    DCR_ASSERT(Colors.size() == Positions.size());

    if (Colors.size() == 0 || Colors.size() == 1)
    {
        return QVector4D(0, 0, 0, 0);
    }

    for (int I = 1; I < Positions.size(); ++I)
    {
        if (Positions[I - 1] <= Parameter && Parameter <= Positions[I])
        {
            const auto S = (Parameter - Positions[I - 1]) / (Positions[I] - Positions[I - 1]);
            return Colors[I - 1] + S * (Colors[I] - Colors[I - 1]);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

QVector4D DiffusionCurveRenderer::Bezier::GetRightColorAt(float Parameter)
{
    const auto& Colors = GetRightColors();
    const auto& Positions = GetRightColorPositions();

    DCR_ASSERT(Colors.size() == Positions.size());

    if (Colors.size() == 0 || Colors.size() == 1)
    {
        return QVector4D(0, 0, 0, 0);
    }

    for (int I = 1; I < Positions.size(); ++I)
    {
        if (Positions[I - 1] <= Parameter && Parameter <= Positions[I])
        {
            const auto S = (Parameter - Positions[I - 1]) / (Positions[I] - Positions[I - 1]);
            return Colors[I - 1] + S * (Colors[I] - Colors[I - 1]);
        }
    }

    return QVector4D(0, 0, 0, 0);
}

QJsonObject DiffusionCurveRenderer::Bezier::ToJsonObject()
{
    QJsonArray ControlPoints;

    for (const auto& pPoint : mControlPoints)
    {
        QJsonObject Object;
        Object.insert("x", pPoint->Position.x());
        Object.insert("y", pPoint->Position.y());
        ControlPoints.append(Object);
    }

    QJsonArray ColorPoints;

    for (const auto& pPoint : mColorPoints)
    {
        QJsonObject Object;
        Object.insert("t", static_cast<int>(pPoint->Type));
        Object.insert("p", pPoint->Position);
        Object.insert("r", pPoint->Color.x());
        Object.insert("g", pPoint->Color.y());
        Object.insert("b", pPoint->Color.z());

        ColorPoints.append(Object);
    }

    QJsonArray BlurPoints;

    for (const auto& pPoint : mBlurPoints)
    {
        QJsonObject Object;
        Object.insert("p", pPoint->Position);
        Object.insert("s", pPoint->Strength);
        BlurPoints.append(Object);
    }

    QJsonObject Object;
    Object.insert("control_points", ControlPoints);
    Object.insert("color_points", ColorPoints);
    Object.insert("blur_points", BlurPoints);

    return Object;
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::Bezier::FromJsonObject(QJsonObject Object)
{
    BezierPtr pBezier = std::make_shared<Bezier>();

    QJsonArray ControlPoints = Object.value("control_points").toArray();
    QJsonArray ColorPoints = Object.value("color_points").toArray();
    QJsonArray BlurPoints = Object.value("blur_points").toArray();

    for (const auto& Point : ControlPoints)
    {
        const auto ControlPoint = Point.toObject();

        if (ControlPoint.isEmpty() == false)
        {
            const auto X = ControlPoint.value("x").toDouble();
            const auto Y = ControlPoint.value("y").toDouble();
            pBezier->AddControlPoint(QVector2D(X, Y));
        }
    }

    for (const auto& Point : ColorPoints)
    {
        const auto ColorPoint = Point.toObject();

        if (ColorPoint.isEmpty() == false)
        {
            const auto Position = ColorPoint.value("p").toDouble();
            const auto R = ColorPoint.value("r").toDouble();
            const auto G = ColorPoint.value("g").toDouble();
            const auto B = ColorPoint.value("b").toDouble();
            const auto Type = ColorPoint.value("t").toInt();

            pBezier->AddColorPoint(static_cast<ColorPointType>(Type), QVector4D(R, G, B, 1.0f), Position);
        }
    }

    for (const auto& Point : BlurPoints)
    {
        const auto BlurPoint = Point.toObject();
        if (BlurPoint.isEmpty() == false)
        {
            const auto Position = BlurPoint.value("p").toDouble();
            const auto Strength = BlurPoint.value("s").toDouble();

            pBezier->AddBlurPoint(Position, Strength);
        }
    }

    return pBezier;
}

std::shared_ptr<DiffusionCurveRenderer::Curve> DiffusionCurveRenderer::Bezier::Clone(const QVector2D& Offset) const
{
    auto pClone = std::make_shared<Bezier>();

    // Copy control points with offset
    for (const auto& pControlPoint : mControlPoints)
    {
        pClone->AddControlPoint(pControlPoint->Position + Offset);
    }

    // Copy color points
    for (const auto& pColorPoint : mColorPoints)
    {
        pClone->AddColorPoint(pColorPoint->Type, pColorPoint->Color, pColorPoint->Position);
    }

    // Copy blur points
    for (const auto& pBlurPoint : mBlurPoints)
    {
        pClone->AddBlurPoint(pBlurPoint->Position, pBlurPoint->Strength);
    }

    // Copy curve properties
    pClone->SetContourColor(GetContourColor());
    pClone->SetContourThickness(GetContourThickness());
    pClone->SetDiffusionWidth(GetDiffusionWidth());
    pClone->SetDiffusionGap(GetDiffusionGap());

    return pClone;
}
