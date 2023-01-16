#include "CurveManager.h"
#include "Common.h"

#include <QtConcurrent>

DiffusionCurveRenderer::CurveManager::CurveManager(QObject* parent)
    : Manager(parent)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
    , mSelectedBlurPoint(nullptr)
{}

bool DiffusionCurveRenderer::CurveManager::Init()
{
    mCamera = EditModeCamera::Instance(); // For fetching zoom, used for color point and blur point selection

    return true;
}

void DiffusionCurveRenderer::CurveManager::AddCurve(Bezier* curve)
{
    mCurves << curve;
    SortCurves();
}

void DiffusionCurveRenderer::CurveManager::AddCurves(const QList<Bezier*> curves)
{
    mCurves << curves;
    SortCurves();
}

void DiffusionCurveRenderer::CurveManager::RemoveCurve(int index)
{
    if (0 <= index && index < mCurves.size())
    {
        Curve* curve = mCurves[index];
        mCurves.removeAt(index);
        delete curve;
        SortCurves();
    }
}

void DiffusionCurveRenderer::CurveManager::RemoveCurve(Curve* curve)
{
    for (int i = 0; i < mCurves.size(); ++i)
    {
        if (mCurves[i] == curve)
        {
            RemoveCurve(i);
            SortCurves();
            return;
        }
    }
}

void DiffusionCurveRenderer::CurveManager::SetGlobalContourThickness(float thickness)
{
    for (auto& curve : mCurves)
        curve->mContourThickness = thickness;
}

void DiffusionCurveRenderer::CurveManager::SetGlobalContourColor(const QVector4D& color)
{
    for (auto& curve : mCurves)
        curve->mContourColor = color;
}

void DiffusionCurveRenderer::CurveManager::SetGlobalDiffusionWidth(float width)
{
    for (auto& curve : mCurves)
        curve->mDiffusionWidth = width;
}

void DiffusionCurveRenderer::CurveManager::SetGlobalDiffusionGap(float gap)
{
    for (auto& curve : mCurves)
        curve->mDiffusionGap = gap;
}

void DiffusionCurveRenderer::CurveManager::SetGlobalBlurStrength(float strength)
{
    for (auto& curve : mCurves)
    {
        auto blurPoints = curve->GetBlurPoints();

        for (auto& blurPoint : blurPoints)
        {
            blurPoint->mStrength = strength;
        }
    }
}

void DiffusionCurveRenderer::CurveManager::DeselectAllCurves()
{
    for (int i = 0; i < mCurves.size(); ++i)
        mCurves[i]->mSelected = false;

    mSelectedCurve = nullptr;
}

void DiffusionCurveRenderer::CurveManager::Select(RenderMode renderMode, const QVector2D& position, float radius)
{
    if (renderMode == RenderMode::Contour)
    {
        if (mSelectedCurve)
        {
            ControlPoint* controlPoint = GetClosestControlPointOnSelectedCurve(position, radius);

            if (controlPoint)
            {
                SetSelectedControlPoint(controlPoint);
                SetSelectedColorPoint(nullptr);
                SetSelectedBlurPoint(nullptr);
                return;
            }
            else
            {
                SetSelectedControlPoint(nullptr);
                SetSelectedColorPoint(nullptr);
                SetSelectedBlurPoint(nullptr);
            }
        }

        SelectCurve(position, radius);
    }
    else
    {
        if (mSelectedCurve)
        {
            ControlPoint* controlPoint = GetClosestControlPointOnSelectedCurve(position, radius);
            ColorPoint* colorPoint = GetClosestColorPointOnSelectedCurve(position, radius);
            BlurPoint* blurPoint = GetClosestBlurPointOnSelectedCurve(position, radius);

            if (controlPoint && colorPoint && blurPoint)
            {
                float distanceToControlPoint = position.distanceToPoint(controlPoint->mPosition);
                float distanceToColorPoint = position.distanceToPoint(colorPoint->GetPosition2D(mCamera->GetZoom() * COLOR_POINT_VISUAL_GAP));
                float distanceToBlurPoint = position.distanceToPoint(blurPoint->GetPosition2D(mCamera->GetZoom() * BLUR_POINT_VISUAL_GAP));

                float min = qMin(distanceToControlPoint, qMin(distanceToColorPoint, distanceToBlurPoint));

                if (qFuzzyCompare(min, distanceToControlPoint))
                {
                    SetSelectedControlPoint(controlPoint);
                    SetSelectedColorPoint(nullptr);
                    SetSelectedBlurPoint(nullptr);
                }

                else if (qFuzzyCompare(min, distanceToColorPoint))
                {
                    SetSelectedColorPoint(colorPoint);
                    SetSelectedControlPoint(nullptr);
                    SetSelectedBlurPoint(nullptr);
                }

                else if (qFuzzyCompare(min, distanceToBlurPoint))
                {
                    SetSelectedBlurPoint(blurPoint);
                    SetSelectedControlPoint(nullptr);
                    SetSelectedColorPoint(nullptr);
                }

                return;
            }
            else if (controlPoint)
            {
                SetSelectedControlPoint(controlPoint);
                SetSelectedColorPoint(nullptr);
                SetSelectedBlurPoint(nullptr);
                return;
            }
            else if (colorPoint)
            {
                SetSelectedColorPoint(colorPoint);
                SetSelectedControlPoint(nullptr);
                SetSelectedBlurPoint(nullptr);
                return;
            }
            else if (blurPoint)
            {
                SetSelectedBlurPoint(blurPoint);
                SetSelectedControlPoint(nullptr);
                SetSelectedColorPoint(nullptr);
                return;
            }
            else
            {
                SetSelectedControlPoint(nullptr);
                SetSelectedColorPoint(nullptr);
                SetSelectedBlurPoint(nullptr);
            }
        }

        SelectCurve(position, radius);
    }
}

void DiffusionCurveRenderer::CurveManager::AddControlPoint(const QVector2D& position, bool select)
{
    if (mSelectedCurve)
    {
        if (mSelectedCurve->GetSize() >= MAX_CONTROL_POINT_COUNT)
            return;

        ControlPoint* controlPoint = new ControlPoint;
        controlPoint->mPosition = position;
        controlPoint->mSelected = true;

        mSelectedCurve->AddControlPoint(controlPoint);

        if (select)
        {
            SetSelectedControlPoint(controlPoint);
            SetSelectedColorPoint(nullptr);
        }
    }
    else
    {
        ControlPoint* controlPoint = new ControlPoint;
        controlPoint->mPosition = position;
        controlPoint->mSelected = true;

        Bezier* curve = new Bezier;
        curve->AddControlPoint(controlPoint);
        AddCurve(curve);
        SetSelectedCurve(curve);

        if (select)
        {
            SetSelectedControlPoint(controlPoint);
            SetSelectedColorPoint(nullptr);
        }
    }
}

void DiffusionCurveRenderer::CurveManager::AddColorPoint(const QVector2D& position, bool select)
{
    if (mSelectedCurve && mSelectedCurve->GetSize() >= 2)
    {
        float parameter = mSelectedCurve->ParameterAt(position);
        QVector3D positionOnCurve = mSelectedCurve->ValueAt(parameter).toVector3D();
        QVector3D tangent = mSelectedCurve->TangentAt(parameter).toVector3D();
        QVector3D direction = (position.toVector3D() - positionOnCurve).normalized();
        QVector3D cross = QVector3D::crossProduct(tangent, direction);

        ColorPoint::Direction type = cross.z() > 0 ? ColorPoint::Direction::Left : ColorPoint::Direction::Right;

        ColorPoint* colorPoint = new ColorPoint;
        colorPoint->SetParent(mSelectedCurve);
        colorPoint->mPosition = parameter;
        colorPoint->mDirection = type;
        colorPoint->mColor = QVector4D(1, 1, 1, 1);
        mSelectedCurve->AddColorPoint(colorPoint);

        if (select)
        {
            SetSelectedControlPoint(nullptr);
            SetSelectedColorPoint(colorPoint);
        }
    }
}

void DiffusionCurveRenderer::CurveManager::AddBlurPoint(const QVector2D& position, bool select)
{
    if (mSelectedCurve && mSelectedCurve->GetSize() >= 2)
    {
        float parameter = mSelectedCurve->ParameterAt(position);

        BlurPoint* blurPoint = new BlurPoint;
        blurPoint->SetParent(mSelectedCurve);
        blurPoint->mPosition = parameter;
        mSelectedCurve->AddBlurPoint(blurPoint);

        if (select)
        {
            SetSelectedControlPoint(nullptr);
            SetSelectedColorPoint(nullptr);
            SetSelectedBlurPoint(blurPoint);
        }
    }
}

void DiffusionCurveRenderer::CurveManager::RemoveSelectedCurve()
{
    if (mSelectedCurve)
    {
        RemoveCurve(mSelectedCurve);
        SetSelectedCurve(nullptr);
        SetSelectedControlPoint(nullptr);
        SetSelectedColorPoint(nullptr);
    }
}

void DiffusionCurveRenderer::CurveManager::RemoveSelectedControlPoint()
{
    if (mSelectedCurve && mSelectedControlPoint)
    {
        mSelectedCurve->RemoveControlPoint(mSelectedControlPoint);
        SetSelectedControlPoint(nullptr);

        if (mSelectedCurve->GetSize() == 0)
        {
            SetSelectedCurve(nullptr);
            RemoveCurve(mSelectedCurve);
        }
    }
}

void DiffusionCurveRenderer::CurveManager::RemoveSelectedColorPoint()
{
    if (mSelectedCurve && mSelectedColorPoint)
    {
        mSelectedCurve->RemoveColorPoint(mSelectedColorPoint);
        SetSelectedColorPoint(nullptr);
    }
}

void DiffusionCurveRenderer::CurveManager::RemoveSelectedBlurPoint()
{
    if (mSelectedCurve && mSelectedBlurPoint)
    {
        mSelectedCurve->RemoveBlurPoint(mSelectedBlurPoint);
        SetSelectedBlurPoint(nullptr);
    }
}

DiffusionCurveRenderer::ControlPoint* DiffusionCurveRenderer::CurveManager::GetClosestControlPointOnSelectedCurve(const QVector2D& nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    ControlPoint* controlPoint = mSelectedCurve->GetClosestControlPoint(nearbyPoint);

    if (controlPoint)
        if (controlPoint->mPosition.distanceToPoint(nearbyPoint) > radius)
            controlPoint = nullptr;

    return controlPoint;
}

DiffusionCurveRenderer::ColorPoint* DiffusionCurveRenderer::CurveManager::GetClosestColorPointOnSelectedCurve(const QVector2D& nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    QVector<ColorPoint*> allColorPoints = mSelectedCurve->GetAllColorPoints();

    if (allColorPoints.size() == 0)
        return nullptr;

    ColorPoint* colorPoint = mSelectedCurve->GetClosestColorPoint(nearbyPoint);

    if (colorPoint)
        if (colorPoint->GetPosition2D(mCamera->GetZoom() * COLOR_POINT_VISUAL_GAP).distanceToPoint(nearbyPoint) > radius)
            colorPoint = nullptr;

    return colorPoint;
}

DiffusionCurveRenderer::BlurPoint* DiffusionCurveRenderer::CurveManager::GetClosestBlurPointOnSelectedCurve(const QVector2D& nearbyPoint, float radius) const
{
    if (!mSelectedCurve)
        return nullptr;

    auto blurPoints = mSelectedCurve->GetBlurPoints();

    if (blurPoints.size() == 0)
        return nullptr;

    BlurPoint* blurPoint = mSelectedCurve->GetClosestBlurPoint(nearbyPoint);

    if (blurPoint)
        if (blurPoint->GetPosition2D(mCamera->GetZoom() * BLUR_POINT_VISUAL_GAP).distanceToPoint(nearbyPoint) > radius)
            blurPoint = nullptr;

    return blurPoint;
}

DiffusionCurveRenderer::CurveManager* DiffusionCurveRenderer::CurveManager::Instance()
{
    static CurveManager instance;
    return &instance;
}

void DiffusionCurveRenderer::CurveManager::SelectCurve(QVector2D position, float radius)
{
    Bezier* curve = nullptr;

    float minDistance = std::numeric_limits<float>::infinity();

    for (int i = 0; i < mCurves.size(); ++i)
    {
        if (mCurves[i]->mVoid)
            continue;

        float distance = mCurves[i]->GetDistanceToPoint(position);
        if (distance < minDistance)
        {
            minDistance = distance;
            curve = mCurves[i];
        }
    }

    if (minDistance < radius)
        SetSelectedCurve(curve);
    else
        SetSelectedCurve(nullptr);
}

DiffusionCurveRenderer::BlurPoint* DiffusionCurveRenderer::CurveManager::GetSelectedBlurPoint() const
{
    return mSelectedBlurPoint;
}

void DiffusionCurveRenderer::CurveManager::SetSelectedBlurPoint(BlurPoint* newSelectedBlurPoint)
{
    if (mSelectedBlurPoint == newSelectedBlurPoint)
        return;

    if (mSelectedBlurPoint)
        mSelectedBlurPoint->mSelected = false;

    if (newSelectedBlurPoint)
        newSelectedBlurPoint->mSelected = true;

    mSelectedBlurPoint = newSelectedBlurPoint;
}

DiffusionCurveRenderer::ColorPoint* DiffusionCurveRenderer::CurveManager::GetSelectedColorPoint() const
{
    return mSelectedColorPoint;
}

void DiffusionCurveRenderer::CurveManager::SetSelectedColorPoint(ColorPoint* newSelectedColorPoint)
{
    if (mSelectedColorPoint == newSelectedColorPoint)
        return;

    if (mSelectedColorPoint)
        mSelectedColorPoint->mSelected = false;

    if (newSelectedColorPoint)
        newSelectedColorPoint->mSelected = true;

    mSelectedColorPoint = newSelectedColorPoint;
}

const QList<DiffusionCurveRenderer::Bezier*>& DiffusionCurveRenderer::CurveManager::GetCurves() const
{
    return mCurves;
}

DiffusionCurveRenderer::Bezier* DiffusionCurveRenderer::CurveManager::GetSelectedCurve() const
{
    return mSelectedCurve;
}

void DiffusionCurveRenderer::CurveManager::SetSelectedCurve(Bezier* newSelectedCurve)
{
    if (mSelectedCurve == newSelectedCurve)
        return;

    if (mSelectedCurve)
        mSelectedCurve->mSelected = false;

    if (newSelectedCurve)
        newSelectedCurve->mSelected = true;

    mSelectedCurve = newSelectedCurve;
}

DiffusionCurveRenderer::ControlPoint* DiffusionCurveRenderer::CurveManager::GetSelectedControlPoint() const
{
    return mSelectedControlPoint;
}

void DiffusionCurveRenderer::CurveManager::SetSelectedControlPoint(ControlPoint* newSelectedControlPoint)
{
    if (mSelectedControlPoint == newSelectedControlPoint)
        return;

    if (mSelectedControlPoint)
        mSelectedControlPoint->mSelected = false;

    if (newSelectedControlPoint)
        newSelectedControlPoint->mSelected = true;

    mSelectedControlPoint = newSelectedControlPoint;
}

void DiffusionCurveRenderer::CurveManager::SortCurves()
{
    if (mCurves.size() == 0 || mCurves.size() == 1)
        return;

    QList<Bezier*> sortedCurves;

    sortedCurves << mCurves[0];

    for (int i = 1; i < mCurves.size(); i++)
    {
        Bezier* curve = mCurves[i];
        if (sortedCurves.last()->mDepth <= curve->mDepth)
            sortedCurves << curve;
        else
            for (int j = 0; j < sortedCurves.size(); j++)
                if (sortedCurves[j]->mDepth > curve->mDepth)
                {
                    sortedCurves.insert(j, curve);
                    break;
                }
    }

    mCurves = sortedCurves;
}

void DiffusionCurveRenderer::CurveManager::Clear()
{
    for (auto& curve : mCurves)
    {
        if (curve)
            delete curve;

        curve = nullptr;
    }

    SetSelectedColorPoint(nullptr);
    SetSelectedControlPoint(nullptr);
    SetSelectedCurve(nullptr);
    mCurves.clear();
}

void DiffusionCurveRenderer::CurveManager::MakeVoid(float threshold)
{
    for (int i = 0; i < mCurves.size(); i++)
    {
        float length = mCurves[i]->GetLength();

        if (length < threshold)
            mCurves[i]->mVoid = true;
        else
            mCurves[i]->mVoid = false;
    }
}