#include "OrthographicCamera.h"

void DiffusionCurveRenderer::OrthographicCamera::Resize(int width, int height, float pixelRatio)
{
    mWidth = width;
    mHeight = height;
    mPixelRatio = pixelRatio;
}

void DiffusionCurveRenderer::OrthographicCamera::OnMousePressed(QMouseEvent* event)
{
    mMouse.x = event->position().x();
    mMouse.y = event->position().y();
    mMouse.dx = 0;
    mMouse.dy = 0;
    mMouse.button = event->button();
}

void DiffusionCurveRenderer::OrthographicCamera::OnMouseReleased(QMouseEvent*)
{
    mMouse.button = Qt::NoButton;
}

void DiffusionCurveRenderer::OrthographicCamera::OnMouseMoved(QMouseEvent* event)
{
    if (mMouse.button == mActionReceiveButton)
    {
        mMouse.dx += mMouse.x - event->pos().x();
        mMouse.dy += mMouse.y - event->pos().y();

        mMouse.x = event->pos().x();
        mMouse.y = event->pos().y();

        mLeft += mPixelRatio * mZoom * mMouse.dx;
        mTop += mPixelRatio * mZoom * mMouse.dy;
        mMouse.dx = 0;
        mMouse.dy = 0;
    }
}

void DiffusionCurveRenderer::OrthographicCamera::OnWheelMoved(QWheelEvent* event)
{
    QPointF cursorWorldPosition = CameraToWorld(event->position());

    if (event->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.001f, qMin(10.0f, mZoom));

    QPointF newWorldPosition = CameraToWorld(event->position());
    QPointF delta = cursorWorldPosition - newWorldPosition;
    mLeft += delta.x();
    mTop += delta.y();
}

const QMatrix4x4& DiffusionCurveRenderer::OrthographicCamera::GetProjectionMatrix()
{
    mProjectionMatrix.setToIdentity();
    mProjectionMatrix.ortho(mLeft, //
                            mLeft + mWidth * mZoom,
                            mTop + mHeight * mZoom,
                            mTop,
                            mZNear,
                            mZFar);
    return mProjectionMatrix;
}

float DiffusionCurveRenderer::OrthographicCamera::GetAspectRatio() const
{
    return mWidth / mHeight;
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::CameraToWorld(const QVector2D& camera)
{
    return CameraToWorld(camera.x(), camera.y());
}

QPointF DiffusionCurveRenderer::OrthographicCamera::CameraToWorld(const QPointF& camera)
{
    return CameraToWorld(camera.x(), camera.y()).toPointF();
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::CameraToWorld(float x, float y)
{
    return QVector2D(mLeft + x * mZoom * mPixelRatio, mTop + y * mZoom * mPixelRatio);
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::WorldToCamera(const QVector2D& world)
{
    return WorldToCamera(world.x(), world.y());
}

QPointF DiffusionCurveRenderer::OrthographicCamera::WorldToCamera(const QPointF& world)
{
    return WorldToCamera(world.x(), world.y()).toPointF();
}

float DiffusionCurveRenderer::OrthographicCamera::WorldDistanceToCameraDistance(float distance)
{
    const auto origin = WorldToCamera(QPointF(0, 0));
    const auto vector = WorldToCamera(QPointF(0, distance));
    const auto delta = vector - origin;

    return std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
}

float DiffusionCurveRenderer::OrthographicCamera::CameraDistanceToWorldDistance(float distance)
{
    const auto origin = CameraToWorld(QPointF(0, 0));
    const auto vector = CameraToWorld(QPointF(0, distance));
    const auto delta = vector - origin;

    return std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
}

void DiffusionCurveRenderer::OrthographicCamera::Reset()
{
    mZoom = 1.0f;
    mTop = 0.0f;
    mLeft = 0.0f;
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::WorldToCamera(float x, float y)
{
    x -= mLeft;
    y -= mTop;

    return QVector2D(x / mZoom, y / mZoom) / mPixelRatio;
}
