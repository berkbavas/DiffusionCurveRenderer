#include "EventHandler.h"

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/RendererManager.h"
#include "Util/Logger.h"


DiffusionCurveRenderer::EventHandler::EventHandler(QObject* parent)
    : QObject(parent)
{
}

void DiffusionCurveRenderer::EventHandler::OnKeyPressed(QKeyEvent* event)
{
    qDebug() << "EventHandler::OnKeyPressed" << event;

    mPressedKey = Qt::Key(event->key());

    if (mPressedKey == Qt::Key_Delete)
    {
        if (mSelectedColorPoint)
        {
            if (mSelectedCurve)
            {
                mSelectedCurve->RemoveColorPoint(mSelectedColorPoint);
                SetSelectedControlPoint(nullptr);
            }
        }
        else if (mSelectedControlPoint)
        {
            if (mSelectedCurve)
            {
                mSelectedCurve->RemoveControlPoint(mSelectedControlPoint);
                SetSelectedControlPoint(nullptr);
            }
        }
        else if (mSelectedCurve)
        {
            mCurveContainer->RemoveCurve(mSelectedCurve);
            SetSelectedCurve(nullptr);
        }
    }
}

void DiffusionCurveRenderer::EventHandler::OnKeyReleased(QKeyEvent* event)
{
    mPressedKey = Qt::Key_No;
}

void DiffusionCurveRenderer::EventHandler::OnMousePressed(QMouseEvent* event)
{
    mMouse.button = event->button();
    mMouse.x = event->position().x();
    mMouse.y = event->position().y();
    mMouse.dx = 0;
    mMouse.dy = 0;

    if (event->button() == Qt::LeftButton)
    {
        CurvePtr curve = GetCurveAround(mMouse.x, mMouse.y);

        if (mSelectedCurve)
        {
            ControlPointPtr control = GetControlPointAround(mMouse.x, mMouse.y);
            ColorPointPtr color = GetColorPointAround(mMouse.x, mMouse.y);

            if (control)
                SetSelectedControlPoint(control);
            else if (color)
                SetSelectedColorPoint(color);
            else
                SetSelectedCurve(curve);
        }
        else
        {
            SetSelectedCurve(curve);
        }
    }
    else if (mMouse.button == Qt::RightButton)
    {

        if (mSelectedCurve)
        {
            if (mPressedKey == Qt::Key_Control)
            {
                if (const auto point = mSelectedCurve->TryCreateColorPointAt(CameraToWorld(mMouse.x, mMouse.y)))
                {
                    if (ColorPointPtr added = mSelectedCurve->AddColorPoint(point->type, point->color, point->position))
                        SelectedColorPointChanged(added);
                }
            }
            else
            {
                if (ControlPointPtr point = mSelectedCurve->AddControlPoint(CameraToWorld(mMouse.x, mMouse.y)))
                    SetSelectedControlPoint(point);
            }
        }
        else
        {
            SplinePtr spline = std::make_shared<Spline>();
            spline->AddControlPoint(CameraToWorld(mMouse.x, mMouse.y));
            mCurveContainer->AddCurve(spline);
            SetSelectedCurve(spline);
        }
    }

    mCamera->OnMousePressed(event);
}

void DiffusionCurveRenderer::EventHandler::OnMouseReleased(QMouseEvent* event)
{
    mMouse.button = Qt::NoButton;

    mCamera->OnMouseReleased(event);
}

void DiffusionCurveRenderer::EventHandler::OnMouseMoved(QMouseEvent* event)
{
    mMouse.dx = event->position().x() - mMouse.x;
    mMouse.dy = event->position().y() - mMouse.y;
    mMouse.x += mMouse.dx;
    mMouse.y += mMouse.dy;

    if (mMouse.button == Qt::LeftButton)
    {

        if (mSelectedControlPoint)
        {
            mSelectedControlPoint->position = CameraToWorld(mMouse.x, mMouse.y);
            mSelectedCurve->Update();
        }
        else if (mSelectedColorPoint)
        {
            const auto newPosition = mSelectedCurve->ParameterAt(CameraToWorld(mMouse.x, mMouse.y));
            const auto colorPoint = mSelectedCurve->AddColorPoint(mSelectedColorPoint->type, mSelectedColorPoint->color, newPosition);
            mSelectedCurve->RemoveColorPoint(mSelectedColorPoint);
            mSelectedCurve->Update();
            SetSelectedColorPoint(colorPoint);
        }
    }
    else if (mMouse.button == Qt::NoButton)
    {
        if (mSelectedCurve != nullptr)
        {
            SetColorPointAround(GetColorPointAround(mMouse.x, mMouse.y));
            SetControlPointAround(GetControlPointAround(mMouse.x, mMouse.y));
        }
    }

    mMouse.x = event->position().x();
    mMouse.y = event->position().y();
    mMouse.dx = 0;
    mMouse.dy = 0;
    mCamera->OnMouseMoved(event);
}

void DiffusionCurveRenderer::EventHandler::OnWheelMoved(QWheelEvent* event)
{
    mCamera->OnWheelMoved(event);
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::EventHandler::GetCurveAround(float x, float y)
{
    CurveQueryInfo info = mRendererManager->Query(QPoint(x, y));

    if (info.result == 1)
    {
        return mCurveContainer->GetCurve(info.index);
    }

    return nullptr;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::EventHandler::GetControlPointAround(float x, float y)
{
    if (mSelectedCurve)
        return mSelectedCurve->FindControlPointAround(CameraToWorld(x, y), CameraDistanceToWorldDistance(HANDLE_OUTER_DISK_RADIUS_PX));

    return nullptr;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::EventHandler::GetColorPointAround(float x, float y)
{
    if (mSelectedCurve)
        return mSelectedCurve->FindColorPointAround(
            CameraToWorld(x, y),
            CameraDistanceToWorldDistance(COLOR_POINT_HANDLE_OFFSET_PX),
            CameraDistanceToWorldDistance(HANDLE_OUTER_DISK_RADIUS_PX));

    return nullptr;
}

QVector2D DiffusionCurveRenderer::EventHandler::CameraToWorld(float x, float y)
{
    return mCamera->CameraToWorld(QVector2D(x, y));
}

float DiffusionCurveRenderer::EventHandler::CameraDistanceToWorldDistance(float distance)
{
    return mCamera->CameraDistanceToWorldDistance(distance);
}

void DiffusionCurveRenderer::EventHandler::SetSelectedCurve(CurvePtr selectedCurve)
{
    if (mSelectedCurve == selectedCurve)
        return;

    mSelectedCurve = selectedCurve;

    SetSelectedControlPoint(nullptr);
    SetSelectedColorPoint(nullptr);
    emit SelectedCurveChanged(selectedCurve);
}

void DiffusionCurveRenderer::EventHandler::SetControlPointAround(ControlPointPtr point)
{
    if (mControlPointAround == point)
        return;

    mControlPointAround = point;
    emit ControlPointAroundChanged(mControlPointAround);
}

void DiffusionCurveRenderer::EventHandler::SetColorPointAround(ColorPointPtr point)
{
    if (mColorPointAround == point)
        return;

    mColorPointAround = point;
    emit ColorPointAroundChanged(mColorPointAround);
}

void DiffusionCurveRenderer::EventHandler::SetSelectedControlPoint(ControlPointPtr point)
{
    if (mSelectedControlPoint == point)
        return;

    mSelectedControlPoint = point;
    SetSelectedColorPoint(nullptr);
    emit SelectedControlPointChanged(mSelectedControlPoint);
}

void DiffusionCurveRenderer::EventHandler::SetSelectedColorPoint(ColorPointPtr point)
{
    if (mSelectedColorPoint == point)
        return;

    mSelectedColorPoint = point;
    SetSelectedControlPoint(nullptr);
    emit SelectedColorPointChanged(mSelectedColorPoint);
}
