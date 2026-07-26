#include "EventHandler.h"

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/RendererManager.h"
#include "Util/Logger.h"

DiffusionCurveRenderer::EventHandler::EventHandler(QObject* pParent)
    : QObject(pParent)
{}

void DiffusionCurveRenderer::EventHandler::OnKeyPressed(QKeyEvent* pEvent)
{
    qDebug() << "EventHandler::OnKeyPressed" << pEvent;

    mPressedKey = Qt::Key(pEvent->key());
    mModifiers = pEvent->modifiers();

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
    // Ctrl+D: Duplicate curve
    else if (mPressedKey == Qt::Key_D && (mModifiers & Qt::ControlModifier))
    {
        if (mSelectedCurve)
        {
            emit DuplicateCurveRequested();
        }
    }
    // F: Zoom to fit
    else if (mPressedKey == Qt::Key_F && !(mModifiers & Qt::ControlModifier))
    {
        emit ZoomToFitRequested();
    }
    // Home: Reset view
    else if (mPressedKey == Qt::Key_Home)
    {
        emit ResetViewRequested();
    }
}

void DiffusionCurveRenderer::EventHandler::OnKeyReleased(QKeyEvent* pEvent)
{
    mPressedKey = Qt::Key_No;
    mModifiers = Qt::NoModifier;
}

void DiffusionCurveRenderer::EventHandler::OnMousePressed(QMouseEvent* pEvent)
{
    mMouse.Button = pEvent->button();
    mMouse.X = pEvent->position().x();
    mMouse.Y = pEvent->position().y();
    mMouse.Dx = 0;
    mMouse.Dy = 0;

    if (pEvent->button() == Qt::LeftButton)
    {
        CurvePtr pCurve = GetCurveAround(mMouse.X, mMouse.Y);

        if (mSelectedCurve)
        {
            ControlPointPtr pControl = GetControlPointAround(mMouse.X, mMouse.Y);
            ColorPointPtr pColor = GetColorPointAround(mMouse.X, mMouse.Y);

            if (pControl)
                SetSelectedControlPoint(pControl);
            else if (pColor)
                SetSelectedColorPoint(pColor);
            else
                SetSelectedCurve(pCurve);
        }
        else
        {
            SetSelectedCurve(pCurve);
        }
    }
    else if (mMouse.Button == Qt::RightButton)
    {
        if (mSelectedCurve)
        {
            if (mPressedKey == Qt::Key_Control)
            {
                if (const auto pPoint = mSelectedCurve->TryCreateColorPointAt(CameraToWorld(mMouse.X, mMouse.Y)))
                {
                    if (ColorPointPtr pAdded = mSelectedCurve->AddColorPoint(pPoint->Type, pPoint->Color, pPoint->Position))
                    {
                        SelectedColorPointChanged(pAdded);
                    }
                }
            }
            else
            {
                if (const auto pPoint = mSelectedCurve->AddControlPoint(CameraToWorld(mMouse.X, mMouse.Y)))
                {
                    SetSelectedControlPoint(pPoint);
                }
            }
        }
        else
        {
            SplinePtr pSpline = std::make_shared<Spline>();
            pSpline->AddControlPoint(CameraToWorld(mMouse.X, mMouse.Y));
            mCurveContainer->AddCurve(pSpline);
            SetSelectedCurve(pSpline);
        }
    }

    mCamera->OnMousePressed(pEvent);
}

void DiffusionCurveRenderer::EventHandler::OnMouseReleased(QMouseEvent* pEvent)
{
    mMouse.Button = Qt::NoButton;

    mCamera->OnMouseReleased(pEvent);
}

void DiffusionCurveRenderer::EventHandler::OnMouseMoved(QMouseEvent* pEvent)
{
    mMouse.Dx = pEvent->position().x() - mMouse.X;
    mMouse.Dy = pEvent->position().y() - mMouse.Y;
    mMouse.X += mMouse.Dx;
    mMouse.Y += mMouse.Dy;

    if (mMouse.Button == Qt::LeftButton)
    {
        if (mSelectedControlPoint)
        {
            mSelectedControlPoint->Position = CameraToWorld(mMouse.X, mMouse.Y);
            mSelectedCurve->Update();
        }
        else if (mSelectedColorPoint)
        {
            const auto NewPosition = mSelectedCurve->ParameterAt(CameraToWorld(mMouse.X, mMouse.Y));
            const auto pColorPoint = mSelectedCurve->AddColorPoint(mSelectedColorPoint->Type, mSelectedColorPoint->Color, NewPosition);
            mSelectedCurve->RemoveColorPoint(mSelectedColorPoint);
            mSelectedCurve->Update();
            SetSelectedColorPoint(pColorPoint);
        }
    }
    else if (mMouse.Button == Qt::NoButton)
    {
        if (mSelectedCurve != nullptr)
        {
            SetColorPointAround(GetColorPointAround(mMouse.X, mMouse.Y));
            SetControlPointAround(GetControlPointAround(mMouse.X, mMouse.Y));
        }
    }

    mMouse.X = pEvent->position().x();
    mMouse.Y = pEvent->position().y();
    mMouse.Dx = 0;
    mMouse.Dy = 0;
    mCamera->OnMouseMoved(pEvent);
}

void DiffusionCurveRenderer::EventHandler::OnWheelMoved(QWheelEvent* pEvent)
{
    mCamera->OnWheelMoved(pEvent);
}

DiffusionCurveRenderer::CurvePtr DiffusionCurveRenderer::EventHandler::GetCurveAround(float X, float Y)
{
    CurveQueryInfo Info = mRendererManager->Query(QPoint(X, Y));

    if (Info.Result == 1)
    {
        return mCurveContainer->GetCurve(Info.Index);
    }

    return nullptr;
}

DiffusionCurveRenderer::ControlPointPtr DiffusionCurveRenderer::EventHandler::GetControlPointAround(float X, float Y)
{
    if (mSelectedCurve)
    {
        return mSelectedCurve->FindControlPointAround(CameraToWorld(X, Y), CameraDistanceToWorldDistance(HANDLE_OUTER_DISK_RADIUS_PX));
    }

    return nullptr;
}

DiffusionCurveRenderer::ColorPointPtr DiffusionCurveRenderer::EventHandler::GetColorPointAround(float X, float Y)
{
    if (mSelectedCurve)
    {
        return mSelectedCurve->FindColorPointAround(CameraToWorld(X, Y), CameraDistanceToWorldDistance(COLOR_POINT_HANDLE_OFFSET_PX), CameraDistanceToWorldDistance(HANDLE_OUTER_DISK_RADIUS_PX));
    }

    return nullptr;
}

QVector2D DiffusionCurveRenderer::EventHandler::CameraToWorld(float X, float Y)
{
    return mCamera->CameraToWorld(QVector2D(X, Y));
}

float DiffusionCurveRenderer::EventHandler::CameraDistanceToWorldDistance(float Distance)
{
    return mCamera->CameraDistanceToWorldDistance(Distance);
}

void DiffusionCurveRenderer::EventHandler::SetSelectedCurve(CurvePtr pSelectedCurve)
{
    if (mSelectedCurve == pSelectedCurve)
    {
        return;
    }

    mSelectedCurve = pSelectedCurve;

    SetSelectedControlPoint(nullptr);
    SetSelectedColorPoint(nullptr);
    emit SelectedCurveChanged(pSelectedCurve);
}

void DiffusionCurveRenderer::EventHandler::SetControlPointAround(ControlPointPtr pPoint)
{
    if (mControlPointAround == pPoint)
    {
        return;
    }

    mControlPointAround = pPoint;
    emit ControlPointAroundChanged(mControlPointAround);
}

void DiffusionCurveRenderer::EventHandler::SetColorPointAround(ColorPointPtr pPoint)
{
    if (mColorPointAround == pPoint)
    {
        return;
    }

    mColorPointAround = pPoint;
    emit ColorPointAroundChanged(mColorPointAround);
}

void DiffusionCurveRenderer::EventHandler::SetSelectedControlPoint(ControlPointPtr pPoint)
{
    if (mSelectedControlPoint == pPoint)
    {
        return;
    }

    mSelectedControlPoint = pPoint;
    SetSelectedColorPoint(nullptr);
    emit SelectedControlPointChanged(mSelectedControlPoint);
}

void DiffusionCurveRenderer::EventHandler::SetSelectedColorPoint(ColorPointPtr pPoint)
{
    if (mSelectedColorPoint == pPoint)
    {
        return;
    }

    mSelectedColorPoint = pPoint;
    SetSelectedControlPoint(nullptr);
    emit SelectedColorPointChanged(mSelectedColorPoint);
}
