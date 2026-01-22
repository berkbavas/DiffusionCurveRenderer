#include "Spline.h"

#include "Util/Logger.h"

#include <limits>

DiffusionCurveRenderer::BezierPtr DiffusionCurveRenderer::Spline::GetBezierPatchAt(float t) const
{
    int index = GetBezierPatchIndexAt(t);

    if (index >= 0)
        return mBezierPatches[index];

    return nullptr;
}

int DiffusionCurveRenderer::Spline::GetBezierPatchIndexAt(float t) const
{
    if (mBezierPatches.size() == 0)
        return -1;

    int index = -1;
    const int numberOfPatches = mBezierPatches.size();
    const float intervalPerPatch = 1.0f / numberOfPatches;

    for (int i = 0; i < numberOfPatches; ++i)
    {
        if (i * intervalPerPatch <= t && t <= (i + 1) * intervalPerPatch)
        {
            index = i;
            break;
        }
    }

    return index;
}

float DiffusionCurveRenderer::Spline::TransformToPatch(float t) const
{
    int numberOfPatches = mBezierPatches.size();
    return t * numberOfPatches - static_cast<int>(t * numberOfPatches);
}

float DiffusionCurveRenderer::Spline::TransformToSpline(int patchIndex, float t) const
{
    const int numberOfPatches = mBezierPatches.size();
    const float intervalPerPatch = 1.0f / numberOfPatches;

    return intervalPerPatch * patchIndex + intervalPerPatch * t;
}

QVector2D DiffusionCurveRenderer::Spline::PositionAt(float t) const
{
    if (const auto patch = GetBezierPatchAt(t))
    {
        return patch->PositionAt(TransformToPatch(t));
    }

    return mControlPoints.first()->position;
}

QVector2D DiffusionCurveRenderer::Spline::TangentAt(float t) const
{
    if (const auto patch = GetBezierPatchAt(t))
        return patch->TangentAt(TransformToPatch(t));
    else
        return QVector2D();
}

QVector2D DiffusionCurveRenderer::Spline::NormalAt(float t) const
{
    if (const auto patch = GetBezierPatchAt(t))
        return patch->NormalAt(TransformToPatch(t));
    else
        return QVector2D();
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Spline::GetControlPoint(int index)
{
    DCR_ASSERT(0 <= index && index < mControlPoints.size());

    return mControlPoints[index];
}

QVector2D DiffusionCurveRenderer::Spline::GetControlPointPosition(int index) const
{
    DCR_ASSERT(0 <= index && index < mControlPoints.size());

    return mControlPoints[index]->position;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::Spline::AddControlPoint(const QVector2D& position)
{
    ControlPointPtr point = std::make_shared<ControlPoint>(position);
    mControlPoints << point;
    mControlPointPositionsDirty = true;
    mIsPointAddedOrRemoved = true;
    Update();
    return point;
}

void DiffusionCurveRenderer::Spline::RemoveControlPoint(ControlPointPtr point)
{
    int index = -1;
    for (int i = 0; i < mControlPoints.size(); ++i)
    {
        if (mControlPoints[i] == point)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        mControlPoints.remove(index);
        mControlPointPositionsDirty = true;
        mIsPointAddedOrRemoved = true;
        Update();
    }
    else
    {
        LOG_WARN("Spline::RemoveControlPoint: ControlPoint could not be removed because it does not belong to this curve.");
    }
}

void DiffusionCurveRenderer::Spline::RemoveControlPoint(int index)
{
    mControlPoints.removeAt(index);
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

        for (int i = 0; i < mControlPoints.size() - 1; ++i)
        {
            mBezierPatches << std::make_shared<Bezier>();
        }
    }

    for (const auto& patch : mBezierPatches)
    {
        patch->RemoveAllControlPoints();
    }

    if (mControlPoints.size() == 2)
    {
        for (int i = 0; i < mControlPoints.size(); ++i)
        {
            mBezierPatches[0]->AddControlPoint(mControlPoints[i]->position);
        }
    }
    else if (mControlPoints.size() == 3)
    {

        for (int i = 0; i < 2; i++)
        {
            const auto cp0 = mControlPoints[i]->position;
            const auto cp1 = (2.0f / 3.0f) * mControlPoints[i]->position + (1.0f / 3.0f) * mControlPoints[i + 1]->position;
            const auto cp2 = (1.0f / 3.0f) * mControlPoints[i]->position + (2.0f / 3.0f) * mControlPoints[i + 1]->position;
            const auto cp3 = mControlPoints[i + 1]->position;

            mBezierPatches[i]->AddControlPoint(cp0);
            mBezierPatches[i]->AddControlPoint(cp1);
            mBezierPatches[i]->AddControlPoint(cp2);
            mBezierPatches[i]->AddControlPoint(cp3);
        }
    }
    else if (mControlPoints.size() >= 4)
    {
        QVector<QVector2D> splineControlPoints = GetSplineControlPoints();

        for (int i = 1; i < mControlPoints.size(); ++i)
        {
            const auto cp0 = mControlPoints[i - 1]->position;
            const auto cp1 = (2.0f / 3.0f) * splineControlPoints[i - 1] + (1.0f / 3.0f) * splineControlPoints[i];
            const auto cp2 = (1.0f / 3.0f) * splineControlPoints[i - 1] + (2.0f / 3.0f) * splineControlPoints[i];
            const auto cp3 = mControlPoints[i]->position;

            mBezierPatches[i - 1]->AddControlPoint(cp0);
            mBezierPatches[i - 1]->AddControlPoint(cp1);
            mBezierPatches[i - 1]->AddControlPoint(cp2);
            mBezierPatches[i - 1]->AddControlPoint(cp3);
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
    int n = mControlPoints.size() - 2;
    Eigen::MatrixXf coef(n, n);

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            coef(i, j) = 0;
        }
    }

    // First row
    coef(0, 0) = 4;
    coef(0, 1) = 1;

    for (int i = 1; i < n - 1; ++i)
    {
        coef(i, i - 1) = 1;
        coef(i, i) = 4;
        coef(i, i + 1) = 1;
    }

    // Last row
    coef(n - 1, n - 2) = 1;
    coef(n - 1, n - 1) = 4;

    return coef;
}

QVector<QVector2D> DiffusionCurveRenderer::Spline::GetSplineControlPoints()
{
    int n = mControlPoints.size();

    Eigen::MatrixXf knotPoints(n, 2);

    for (int i = 0; i < n; ++i)
    {
        knotPoints(i, 0) = mControlPoints[i]->position.x();
        knotPoints(i, 1) = mControlPoints[i]->position.y();
    }

    // Constants on the right side
    Eigen::MatrixXf constants(n - 2, 2);

    for (int j = 0; j < 2; ++j)
    {
        constants(0, j) = 6 * knotPoints(1, j) - knotPoints(0, j);
        constants(n - 3, j) = 6 * knotPoints(n - 2, j) - knotPoints(n - 1, j);
    }

    for (int i = 1; i < n - 3; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            constants(i, j) = 6 * knotPoints(i + 1, j);
        }
    }

    // Compute B-Spline control points
    Eigen::MatrixXf coef = CreateCoefficientMatrix();
    Eigen::MatrixXf controlPoints = coef.inverse() * constants;

    // Result
    QVector<QVector2D> result(n, QVector2D());
    result[0] = QVector2D(knotPoints(0, 0), knotPoints(0, 1));
    result[n - 1] = QVector2D(knotPoints(n - 1, 0), knotPoints(n - 1, 1));
    for (int i = 0; i < n - 2; ++i)
    {
        result[i + 1] = QVector2D(controlPoints(i, 0), controlPoints(i, 1));
    }

    return result;
}

void DiffusionCurveRenderer::Spline::SaveColorPoints()
{
    mColorsBeforeUpdate.clear();
    for (int index = 0; index < mBezierPatches.size(); ++index)
    {
        const auto& patchColors = mBezierPatches[index]->GetColorPoints();
        for (const auto color : patchColors)
        {
            ColorPointPtr point = std::make_shared<ColorPoint>(color->type, color->color, TransformToSpline(index, color->position));
            mColorsBeforeUpdate << point;
        }
    }
}

void DiffusionCurveRenderer::Spline::RestoreColorPoints()
{
    for (const auto color : mColorsBeforeUpdate)
    {
        AddColorPoint(color->type, color->color, color->position);
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
            mControlPointPositions << controlPoint->position;
        }

        mControlPointPositionsDirty = false;
    }

    return mControlPointPositions;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Spline::AddColorPoint(ColorPointType type, const QVector4D& color, float position)
{
    if (BezierPtr patch = GetBezierPatchAt(position))
    {
        const auto transformed = TransformToPatch(position);
        return patch->AddColorPoint(type, color, transformed);
    }

    return nullptr;
}

bool DiffusionCurveRenderer::Spline::RemoveColorPoint(ColorPointPtr point)
{
    for (const auto& patch : mBezierPatches)
    {
        if (patch->RemoveColorPoint(point))
            return true;
    }

    qWarning() << "Spline::RemoveColorPoint: ColorPoint could not be removed because it does not belong to any Bezier patches.";
    return false;
}

DiffusionCurveRenderer::BlurPointPtr DiffusionCurveRenderer::Spline::AddBlurPoint(float position, float strength)
{
    if (BezierPtr patch = GetBezierPatchAt(position))
    {
        return patch->AddBlurPoint(position, strength);
    }

    return nullptr;
}

bool DiffusionCurveRenderer::Spline::RemoveBlurPoint(BlurPointPtr point)
{
    for (const auto& patch : mBezierPatches)
    {
        if (patch->RemoveBlurPoint(point))
            return true;
    }

    qWarning() << "Spline::RemoveBlurPoint: BlurPoint could not be removed because it does not belong to any Bezier patches.";
    return false;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::Spline::FindColorPointAround(const QVector2D& test, float offset, float tolerance)
{
    for (const auto& patch : mBezierPatches)
    {
        if (const auto colorPoint = patch->FindColorPointAround(test, offset, tolerance))
        {
            return colorPoint;
        }
    }

    return nullptr;
}

QJsonObject DiffusionCurveRenderer::Spline::ToJsonObject()
{
    QJsonArray controlPoints;

    for (const auto& point : mControlPoints)
    {
        QJsonObject object;
        object.insert("x", point->position.x());
        object.insert("y", point->position.y());
        controlPoints.append(object);
    }

    QJsonArray patches;

    for (const auto& patch : mBezierPatches)
    {
        patches.append(patch->ToJsonObject());
    }

    QJsonObject object;
    object.insert("control_points", controlPoints);
    object.insert("patches", patches);

    return object;
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::Spline::FromJsonObject(QJsonObject object)
{
    SplinePtr spline = std::make_shared<Spline>();

    QJsonArray controlPoints = object.value("control_points").toArray();

    for (const auto& point : controlPoints)
    {
        const auto controlPoint = point.toObject();

        if (controlPoint.isEmpty() == false)
        {
            float x = controlPoint.value("x").toDouble();
            float y = controlPoint.value("y").toDouble();
            spline->mControlPoints << std::make_shared<ControlPoint>(QVector2D(x, y));
        }
    }

    QJsonArray patches = object.value("patches").toArray();

    for (const auto& patch : patches)
    {
        const auto patchObject = patch.toObject();

        if (patchObject.isEmpty() == false)
        {
            CurvePtr bezier = Bezier::FromJsonObject(patchObject);
            spline->mBezierPatches << std::dynamic_pointer_cast<Bezier>(bezier);
        }
    }

    spline->mIsPointAddedOrRemoved = true;
    spline->Update();

    return spline;
}

std::shared_ptr<DiffusionCurveRenderer::Curve> DiffusionCurveRenderer::Spline::Clone(const QVector2D& offset) const
{
    auto clone = std::make_shared<Spline>();
    
    // Copy control points with offset
    for (const auto& controlPoint : mControlPoints)
    {
        clone->AddControlPoint(controlPoint->position + offset);
    }
    
    // Need to update to create patches
    clone->Update();
    
    // Copy color points from patches
    for (int i = 0; i < mBezierPatches.size() && i < clone->mBezierPatches.size(); ++i)
    {
        const auto& srcPatch = mBezierPatches[i];
        auto& dstPatch = clone->mBezierPatches[i];
        
        for (const auto& colorPoint : srcPatch->GetColorPoints())
        {
            dstPatch->AddColorPoint(colorPoint->type, colorPoint->color, colorPoint->position);
        }
        
        for (const auto& blurPoint : srcPatch->GetBlurPoints())
        {
            dstPatch->AddBlurPoint(blurPoint->position, blurPoint->strength);
        }
    }
    
    // Copy curve properties
    clone->SetContourColor(GetContourColor());
    clone->SetContourThickness(GetContourThickness());
    clone->SetDiffusionWidth(GetDiffusionWidth());
    clone->SetDiffusionGap(GetDiffusionGap());
    
    return clone;
}
