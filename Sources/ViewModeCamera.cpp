#include "ViewModeCamera.h"

DiffusionCurveRenderer::ViewModeCamera::ViewModeCamera(QObject* parent)
    : QObject(parent)
    , mZNear(-1)
    , mZFar(1)
    , mZoom(4.0f)
    , mLeft(0)
    , mTop(0)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)
{}

void DiffusionCurveRenderer::ViewModeCamera::OnMousePressed(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mMouse.pressed = true;
    }
}

void DiffusionCurveRenderer::ViewModeCamera::OnMouseReleased(QMouseEvent* event)
{
    mMouse.pressed = false;
}

void DiffusionCurveRenderer::ViewModeCamera::OnMouseMoved(QMouseEvent* event)
{
    if (mMouse.pressed)
    {
        mMouse.dx += mMouse.x - event->position().x();
        mMouse.dy += mMouse.y - event->position().y();

        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mUpdatePosition = true;
    }
}

void DiffusionCurveRenderer::ViewModeCamera::OnWheelMoved(QWheelEvent* event)
{
    if (event->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.25f, qMin(10.0f, mZoom));
}

void DiffusionCurveRenderer::ViewModeCamera::Resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

void DiffusionCurveRenderer::ViewModeCamera::Update(float ifps)
{
    if (mUpdatePosition)
    {
        mLeft -= mMouse.dx * mZoom;
        mTop += mMouse.dy * mZoom;
        mMouse.dx = 0;
        mMouse.dy = 0;
        mUpdatePosition = false;
    }
}

QMatrix4x4 DiffusionCurveRenderer::ViewModeCamera::GetProjection() const
{
    QMatrix4x4 projection;

    projection.ortho(-mZoom * mWidth / 2 - mLeft, //
        mZoom * mWidth / 2 - mLeft,
        -mZoom * mHeight / 2 - mTop,
        mZoom * mHeight / 2 - mTop,
        mZNear,
        mZFar);

    return projection;
}

float DiffusionCurveRenderer::ViewModeCamera::GetPixelRatio() const
{
    return mPixelRatio;
}

void DiffusionCurveRenderer::ViewModeCamera::SetPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}

float DiffusionCurveRenderer::ViewModeCamera::GetLeft() const
{
    return mLeft;
}

void DiffusionCurveRenderer::ViewModeCamera::SetLeft(float newLeft)
{
    mLeft = newLeft;
}

float DiffusionCurveRenderer::ViewModeCamera::GetTop() const
{
    return mTop;
}

void DiffusionCurveRenderer::ViewModeCamera::SetTop(float newTop)
{
    mTop = newTop;
}

float DiffusionCurveRenderer::ViewModeCamera::GetWidth() const
{
    return mWidth;
}

float DiffusionCurveRenderer::ViewModeCamera::GetHeight() const
{
    return mHeight;
}

float DiffusionCurveRenderer::ViewModeCamera::GetZoom() const
{
    return mZoom;
}

void DiffusionCurveRenderer::ViewModeCamera::SetZoom(float newZoom)
{
    mZoom = newZoom;
}

DiffusionCurveRenderer::ViewModeCamera* DiffusionCurveRenderer::ViewModeCamera::Instance()
{
    static ViewModeCamera instance;

    return &instance;
}