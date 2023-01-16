#include "EditModeCamera.h"

#include <QMatrix4x4>

DiffusionCurveRenderer::EditModeCamera::EditModeCamera(QObject* parent)
    : QObject(parent)
    , mZNear(-1)
    , mZFar(1)
    , mZoom(1.0f)
    , mLeft(0)
    , mTop(0)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)
{}

void DiffusionCurveRenderer::EditModeCamera::OnMousePressed(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        mMouse.x = event->position().x();
        mMouse.y = event->position().y();
        mMouse.pressed = true;
    }
}

void DiffusionCurveRenderer::EditModeCamera::OnMouseReleased(QMouseEvent* event)
{
    mMouse.pressed = false;
}

void DiffusionCurveRenderer::EditModeCamera::OnMouseMoved(QMouseEvent* event)
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

void DiffusionCurveRenderer::EditModeCamera::OnWheelMoved(QWheelEvent* event)
{
    QVector2D cursorWorldPosition = ToOpenGL(event->position());

    if (event->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.001f, qMin(10.0f, mZoom));

    QVector2D newWorldPosition = ToOpenGL(event->position());
    QVector2D delta = cursorWorldPosition - newWorldPosition;
    mLeft += delta.x();
    mTop += delta.y();
}

void DiffusionCurveRenderer::EditModeCamera::Resize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

void DiffusionCurveRenderer::EditModeCamera::Update(float ifps)
{
    if (mUpdatePosition)
    {
        mLeft += mZoom * mMouse.dx;
        mTop += mZoom * mMouse.dy;
        mMouse.dx = 0;
        mMouse.dy = 0;
        mUpdatePosition = false;
    }
}

QMatrix4x4 DiffusionCurveRenderer::EditModeCamera::GetProjection() const
{
    QMatrix4x4 projection;

    projection.ortho(mLeft, //
        mLeft + mWidth * mZoom,
        mTop + mHeight * mZoom,
        mTop,
        -1,
        1);

    return projection;
}

QVector2D DiffusionCurveRenderer::EditModeCamera::ToOpenGL(const QPointF& position) const
{
    return QVector2D(mLeft + mZoom * position.x(), mTop + mZoom * position.y());
}

QPointF DiffusionCurveRenderer::EditModeCamera::ToGUI(const QPointF& position) const
{
    float x = position.x() - mLeft;
    float y = position.y() - mTop;

    return QPointF(x * mPixelRatio / mZoom, y * mPixelRatio / mZoom);
}

QPointF DiffusionCurveRenderer::EditModeCamera::ToGUI(const QVector2D& position) const
{
    return ToGUI(position.toPointF());
}

QRectF DiffusionCurveRenderer::EditModeCamera::ToGUI(const QRectF& rect) const
{
    float w = rect.width() * mPixelRatio / mZoom;
    float h = rect.height() * mPixelRatio / mZoom;

    QPointF center = ToGUI(QPointF(rect.center().x(), rect.center().y()));

    return QRectF(center.x() - 0.5 * w, center.y() - 0.5 * h, w, h);
}

float DiffusionCurveRenderer::EditModeCamera::GetPixelRatio() const
{
    return mPixelRatio;
}

void DiffusionCurveRenderer::EditModeCamera::SetPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}

float DiffusionCurveRenderer::EditModeCamera::GetLeft() const
{
    return mLeft;
}

void DiffusionCurveRenderer::EditModeCamera::SetLeft(float newLeft)
{
    mLeft = newLeft;
}

float DiffusionCurveRenderer::EditModeCamera::GetTop() const
{
    return mTop;
}

void DiffusionCurveRenderer::EditModeCamera::SetTop(float newTop)
{
    mTop = newTop;
}

float DiffusionCurveRenderer::EditModeCamera::GetWidth() const
{
    return mWidth;
}

float DiffusionCurveRenderer::EditModeCamera::GetHeight() const
{
    return mHeight;
}

float DiffusionCurveRenderer::EditModeCamera::GetZoom() const
{
    return mZoom;
}

void DiffusionCurveRenderer::EditModeCamera::SetZoom(float newZoom)
{
    mZoom = newZoom;
}

DiffusionCurveRenderer::EditModeCamera* DiffusionCurveRenderer::EditModeCamera::Instance()
{
    static EditModeCamera instance;

    return &instance;
}