#include "Spline.h"

#include "Util/Logger.h"

#include <limits>

DiffusionCurveRenderer::BezierPtr DiffusionCurveRenderer::Spline::GetBezierPatchAt(float Parameter) const
{
    const auto Index = GetBezierPatchIndexAt(Parameter);

    if (Index >= 0)
    {
        return mBezierPatches[Index];
    }

    return nullptr;
}

int DiffusionCurveRenderer::Spline::GetBezierPatchIndexAt(float Parameter) const
{
    if (mBezierPatches.size() == 0)
    {
        return -1;
    }

    int Index = -1;
    const int NumberOfPatches = mBezierPatches.size();
    const float IntervalPerPatch = 1.0f / NumberOfPatches;

    for (int I = 0; I < NumberOfPatches; ++I)
    {
        if (I * IntervalPerPatch <= Parameter && Parameter <= (I + 1) * IntervalPerPatch)
        {
            Index = I;
            break;
        }
    }

    return Index;
}

float DiffusionCurveRenderer::Spline::TransformToPatch(float Parameter) const
{
    int NumberOfPatches = mBezierPatches.size();
    return Parameter * NumberOfPatches - static_cast<int>(Parameter * NumberOfPatches);
}

float DiffusionCurveRenderer::Spline::TransformToSpline(int PatchIndex, float Parameter) const
{
    const int NumberOfPatches = mBezierPatches.size();
    const float IntervalPerPatch = 1.0f / NumberOfPatches;

    return IntervalPerPatch * PatchIndex + IntervalPerPatch * Parameter;
}

QVector2D DiffusionCurveRenderer::Spline::PositionAt(float Parameter) const
{
    if (const auto pPatch = GetBezierPatchAt(Parameter))
    {
        return pPatch->PositionAt(TransformToPatch(Parameter));
    }

    return mControlPoints.first()->Position;
}

QVector2D DiffusionCurveRenderer::Spline::TangentAt(float Parameter) const
{
    if (const auto pPatch = GetBezierPatchAt(Parameter))
        return pPatch->TangentAt(TransformToPatch(Parameter));
    else
        return QVector2D();
}

QVector2D DiffusionCurveRenderer::Spline::NormalAt(float Parameter) const
{
    if (const auto pPatch = GetBezierPatchAt(Parameter))
        return pPatch->NormalAt(TransformToPatch(Parameter));
    else
        return QVector2D();
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Spline::GetControlPoint(int Index)
{
    DCR_ASSERT(0 <= Index && Index < mControlPoints.size());

    return mControlPoints[Index];
}

QVector2D DiffusionCurveRenderer::Spline::GetControlPointPosition(int Index) const
{
    DCR_ASSERT(0 <= Index && Index < mControlPoints.size());

    return mControlPoints[Index]->Position;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Spline::AddControlPoint(const QVector2D& Position)
{
    ControlPointPtr pPoint = std::make_shared<ControlPoint>(Position);
    mControlPoints << pPoint;
    mControlPointPositionsDirty = true;
    mIsPointAddedOrRemoved = true;
    Update();
    return pPoint;
}

void DiffusionCurveRenderer::Spline::RemoveControlPoint(ControlPointPtr pPoint)
{
    int Index = -1;
    for (int I = 0; I < mControlPoints.size(); ++I)
    {
        if (mControlPoints[I] == pPoint)
        {
            Index = I;
            break;
        }
    }

    if (Index != -1)
    {
        mControlPoints.remove(Index);
        mControlPointPositionsDirty = true;
        mIsPointAddedOrRemoved = true;
        Update();
    }
    else
    {
        LOG_WARN("Spline::RemoveControlPoint: ControlPoint could not be removed because it does not belong to this curve.");
    }
}

void DiffusionCurveRenderer::Spline::RemoveControlPoint(int Index)
{
    mControlPoints.removeAt(Index);
    mControlPointPositionsDirty = true;
    mIsPointAddedOrRemoved = true;
    Update();
}

void DiffusionCurveRenderer::Spline::Update()
{
    if (mIsPointAddedOrRemoved)
    {
        SaveColorPoints();

        mBezierPatches.clear();

        for (int I = 0; I < mControlPoints.size() - 1; ++I)
        {
            mBezierPatches << std::make_shared<Bezier>();
        }
    }

    for (const auto& pPatch : mBezierPatches)
    {
        pPatch->RemoveAllControlPoints();
    }

    if (mControlPoints.size() == 2)
    {
        for (int I = 0; I < mControlPoints.size(); ++I)
        {
            mBezierPatches[0]->AddControlPoint(mControlPoints[I]->Position);
        }
    }
    else if (mControlPoints.size() == 3)
    {
        for (int I = 0; I < 2; I++)
        {
            const auto CP0 = mControlPoints[I]->Position;
            const auto CP1 = (2.0f / 3.0f) * mControlPoints[I]->Position + (1.0f / 3.0f) * mControlPoints[I + 1]->Position;
            const auto CP2 = (1.0f / 3.0f) * mControlPoints[I]->Position + (2.0f / 3.0f) * mControlPoints[I + 1]->Position;
            const auto CP3 = mControlPoints[I + 1]->Position;

            mBezierPatches[I]->AddControlPoint(CP0);
            mBezierPatches[I]->AddControlPoint(CP1);
            mBezierPatches[I]->AddControlPoint(CP2);
            mBezierPatches[I]->AddControlPoint(CP3);
        }
    }
    else if (mControlPoints.size() >= 4)
    {
        QVector<QVector2D> splineControlPoints = GetSplineControlPoints();

        for (int I = 1; I < mControlPoints.size(); ++I)
        {
            const auto CP0 = mControlPoints[I - 1]->Position;
            const auto CP1 = (2.0f / 3.0f) * splineControlPoints[I - 1] + (1.0f / 3.0f) * splineControlPoints[I];
            const auto CP2 = (1.0f / 3.0f) * splineControlPoints[I - 1] + (2.0f / 3.0f) * splineControlPoints[I];
            const auto CP3 = mControlPoints[I]->Position;

            mBezierPatches[I - 1]->AddControlPoint(CP0);
            mBezierPatches[I - 1]->AddControlPoint(CP1);
            mBezierPatches[I - 1]->AddControlPoint(CP2);
            mBezierPatches[I - 1]->AddControlPoint(CP3);
        }
    }

    if (mIsPointAddedOrRemoved)
    {
        RestoreColorPoints();
        mIsPointAddedOrRemoved = false;
    }
}

Eigen::MatrixXf DiffusionCurveRenderer::Spline::CreateCoefficientMatrix()
{
    const auto N = mControlPoints.size() - 2;
    Eigen::MatrixXf Coef(N, N);

    for (int I = 0; I < N; ++I)
    {
        for (int J = 0; J < N; ++J)
        {
            Coef(I, J) = 0;
        }
    }

    // First row
    Coef(0, 0) = 4;
    Coef(0, 1) = 1;

    for (int I = 1; I < N - 1; ++I)
    {
        Coef(I, I - 1) = 1;
        Coef(I, I) = 4;
        Coef(I, I + 1) = 1;
    }

    // Last row
    Coef(N - 1, N - 2) = 1;
    Coef(N - 1, N - 1) = 4;

    return Coef;
}

QVector<QVector2D> DiffusionCurveRenderer::Spline::GetSplineControlPoints()
{
    const auto N = mControlPoints.size();

    Eigen::MatrixXf KnotPoints(N, 2);

    for (int I = 0; I < N; ++I)
    {
        KnotPoints(I, 0) = mControlPoints[I]->Position.x();
        KnotPoints(I, 1) = mControlPoints[I]->Position.y();
    }

    // Constants on the right side
    Eigen::MatrixXf Constants(N - 2, 2);

    for (int J = 0; J < 2; ++J)
    {
        Constants(0, J) = 6 * KnotPoints(1, J) - KnotPoints(0, J);
        Constants(N - 3, J) = 6 * KnotPoints(N - 2, J) - KnotPoints(N - 1, J);
    }

    for (int I = 1; I < N - 3; ++I)
    {
        for (int J = 0; J < 2; ++J)
        {
            Constants(I, J) = 6 * KnotPoints(I + 1, J);
        }
    }

    // Compute B-Spline control points
    Eigen::MatrixXf Coef = CreateCoefficientMatrix();
    Eigen::MatrixXf ControlPoints = Coef.inverse() * Constants;

    QVector<QVector2D> Result(N, QVector2D());
    Result[0] = QVector2D(KnotPoints(0, 0), KnotPoints(0, 1));
    Result[N - 1] = QVector2D(KnotPoints(N - 1, 0), KnotPoints(N - 1, 1));
    for (int I = 0; I < N - 2; ++I)
    {
        Result[I + 1] = QVector2D(ControlPoints(I, 0), ControlPoints(I, 1));
    }

    return Result;
}

void DiffusionCurveRenderer::Spline::SaveColorPoints()
{
    mColorsBeforeUpdate.clear();
    for (int Index = 0; Index < mBezierPatches.size(); ++Index)
    {
        const auto& PatchColors = mBezierPatches[Index]->GetColorPoints();
        for (const auto pColor : PatchColors)
        {
            ColorPointPtr pPoint = std::make_shared<ColorPoint>(pColor->Type, pColor->Color, TransformToSpline(Index, pColor->Position));
            mColorsBeforeUpdate << pPoint;
        }
    }
}

void DiffusionCurveRenderer::Spline::RestoreColorPoints()
{
    for (const auto& pColor : mColorsBeforeUpdate)
    {
        AddColorPoint(pColor->Type, pColor->Color, pColor->Position);
    }

    mColorsBeforeUpdate.clear();
}

const QVector<QVector2D>& DiffusionCurveRenderer::Spline::GetControlPointPositions()
{
    if (mControlPointPositionsDirty)
    {
        mControlPointPositions.clear();

        for (const auto& controlPoint : mControlPoints)
        {
            mControlPointPositions << controlPoint->Position;
        }

        mControlPointPositionsDirty = false;
    }

    return mControlPointPositions;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Spline::AddColorPoint(ColorPointType Type, const QVector4D& Color, float Position)
{
    if (BezierPtr pPatch = GetBezierPatchAt(Position))
    {
        const auto Transformed = TransformToPatch(Position);
        return pPatch->AddColorPoint(Type, Color, Transformed);
    }

    return nullptr;
}

bool DiffusionCurveRenderer::Spline::RemoveColorPoint(ColorPointPtr pPoint)
{
    for (const auto& pPatch : mBezierPatches)
    {
        if (pPatch->RemoveColorPoint(pPoint))
            return true;
    }

    qWarning() << "Spline::RemoveColorPoint: ColorPoint could not be removed because it does not belong to any Bezier patches.";
    return false;
}

DiffusionCurveRenderer::BlurPointPtr DiffusionCurveRenderer::Spline::AddBlurPoint(float Position, float Strength)
{
    if (BezierPtr pPatch = GetBezierPatchAt(Position))
    {
        return pPatch->AddBlurPoint(Position, Strength);
    }

    return nullptr;
}

bool DiffusionCurveRenderer::Spline::RemoveBlurPoint(BlurPointPtr pPoint)
{
    for (const auto& pPatch : mBezierPatches)
    {
        if (pPatch->RemoveBlurPoint(pPoint))
            return true;
    }

    qWarning() << "Spline::RemoveBlurPoint: BlurPoint could not be removed because it does not belong to any Bezier patches.";
    return false;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Spline::FindColorPointAround(const QVector2D& Test, float Offset, float Tolerance)
{
    for (const auto& pPatch : mBezierPatches)
    {
        if (const auto pColorPoint = pPatch->FindColorPointAround(Test, Offset, Tolerance))
        {
            return pColorPoint;
        }
    }

    return nullptr;
}

QJsonObject DiffusionCurveRenderer::Spline::ToJsonObject()
{
    QJsonArray ControlPoints;

    for (const auto& pPoint : mControlPoints)
    {
        QJsonObject Object;
        Object.insert("x", pPoint->Position.x());
        Object.insert("y", pPoint->Position.y());
        ControlPoints.append(Object);
    }

    QJsonArray Patches;

    for (const auto& pPatch : mBezierPatches)
    {
        Patches.append(pPatch->ToJsonObject());
    }

    QJsonObject Object;
    Object.insert("control_points", ControlPoints);
    Object.insert("patches", Patches);

    return Object;
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::Spline::FromJsonObject(QJsonObject Object)
{
    SplinePtr pSpline = std::make_shared<Spline>();

    QJsonArray ControlPoints = Object.value("control_points").toArray();

    for (const auto& pPoint : ControlPoints)
    {
        const auto pControlPoint = pPoint.toObject();

        if (pControlPoint.isEmpty() == false)
        {
            const auto X = pControlPoint.value("x").toDouble();
            const auto Y = pControlPoint.value("y").toDouble();
            pSpline->mControlPoints << std::make_shared<ControlPoint>(QVector2D(X, Y));
        }
    }

    QJsonArray Patches = Object.value("patches").toArray();

    for (const auto& pPatch : Patches)
    {
        const auto PatchObject = pPatch.toObject();

        if (PatchObject.isEmpty() == false)
        {
            const auto pBezier = Bezier::FromJsonObject(PatchObject);
            pSpline->mBezierPatches << std::dynamic_pointer_cast<Bezier>(pBezier);
        }
    }

    pSpline->mIsPointAddedOrRemoved = true;
    pSpline->Update();

    return pSpline;
}

std::shared_ptr<DiffusionCurveRenderer::Curve> DiffusionCurveRenderer::Spline::Clone(const QVector2D& Offset) const
{
    auto pClone = std::make_shared<Spline>();

    // Copy control points with offset
    for (const auto& pControlPoint : mControlPoints)
    {
        pClone->AddControlPoint(pControlPoint->Position + Offset);
    }

    // Need to update to create patches
    pClone->Update();

    // Copy color points from patches
    for (int i = 0; i < mBezierPatches.size() && i < pClone->mBezierPatches.size(); ++i)
    {
        const auto& pSrcPatch = mBezierPatches[i];
        auto& pDstPatch = pClone->mBezierPatches[i];

        for (const auto& colorPoint : pSrcPatch->GetColorPoints())
        {
            pDstPatch->AddColorPoint(colorPoint->Type, colorPoint->Color, colorPoint->Position);
        }

        for (const auto& blurPoint : pSrcPatch->GetBlurPoints())
        {
            pDstPatch->AddBlurPoint(blurPoint->Position, blurPoint->Strength);
        }
    }

    // Copy curve properties
    pClone->SetContourColor(GetContourColor());
    pClone->SetContourThickness(GetContourThickness());
    pClone->SetDiffusionWidth(GetDiffusionWidth());
    pClone->SetDiffusionGap(GetDiffusionGap());

    return pClone;
}
