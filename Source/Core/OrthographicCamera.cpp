#include "OrthographicCamera.h"

void DiffusionCurveRenderer::OrthographicCamera::Resize(int Width, int Height, float PixelRatio)
{
    mWidth = Width;
    mHeight = Height;
    mPixelRatio = PixelRatio;
}

void DiffusionCurveRenderer::OrthographicCamera::OnMousePressed(QMouseEvent* pEvent)
{
    mMouse.X = pEvent->position().x();
    mMouse.Y = pEvent->position().y();
    mMouse.Dx = 0;
    mMouse.Dy = 0;
    mMouse.Button = pEvent->button();
}

void DiffusionCurveRenderer::OrthographicCamera::OnMouseReleased(QMouseEvent* pEvent)
{
    mMouse.Button = Qt::NoButton;
}

void DiffusionCurveRenderer::OrthographicCamera::OnMouseMoved(QMouseEvent* pEvent)
{
    if (mMouse.Button == mActionReceiveButton)
    {
        mMouse.Dx += mMouse.X - pEvent->pos().x();
        mMouse.Dy += mMouse.Y - pEvent->pos().y();

        mMouse.X = pEvent->pos().x();
        mMouse.Y = pEvent->pos().y();

        mLeft += mPixelRatio * mZoom * mMouse.Dx;
        mTop += mPixelRatio * mZoom * mMouse.Dy;
        mMouse.Dx = 0;
        mMouse.Dy = 0;
    }
}

void DiffusionCurveRenderer::OrthographicCamera::OnWheelMoved(QWheelEvent* pEvent)
{
    const auto CursorWorldPosition = CameraToWorld(pEvent->position());

    if (pEvent->angleDelta().y() < 0)
        mZoom = 1.1f * mZoom;
    else
        mZoom = mZoom / 1.1f;

    mZoom = qMax(0.001f, qMin(10.0f, mZoom));

    const auto NewWorldPosition = CameraToWorld(pEvent->position());
    const auto Delta = CursorWorldPosition - NewWorldPosition;
    mLeft += Delta.x();
    mTop += Delta.y();
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

QVector2D DiffusionCurveRenderer::OrthographicCamera::CameraToWorld(const QVector2D& Camera)
{
    return CameraToWorld(Camera.x(), Camera.y());
}

QPointF DiffusionCurveRenderer::OrthographicCamera::CameraToWorld(const QPointF& Camera)
{
    return CameraToWorld(Camera.x(), Camera.y()).toPointF();
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::CameraToWorld(float X, float Y)
{
    return QVector2D(mLeft + X * mZoom * mPixelRatio, mTop + Y * mZoom * mPixelRatio);
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::WorldToCamera(const QVector2D& World)
{
    return WorldToCamera(World.x(), World.y());
}

QPointF DiffusionCurveRenderer::OrthographicCamera::WorldToCamera(const QPointF& World)
{
    return WorldToCamera(World.x(), World.y()).toPointF();
}

float DiffusionCurveRenderer::OrthographicCamera::WorldDistanceToCameraDistance(float Distance)
{
    const auto Origin = WorldToCamera(QPointF(0, 0));
    const auto Vector = WorldToCamera(QPointF(0, Distance));
    const auto Delta = Vector - Origin;

    return std::sqrt(Delta.x() * Delta.x() + Delta.y() * Delta.y());
}

float DiffusionCurveRenderer::OrthographicCamera::CameraDistanceToWorldDistance(float Distance)
{
    const auto Origin = CameraToWorld(QPointF(0, 0));
    const auto Vector = CameraToWorld(QPointF(0, Distance));
    const auto Delta = Vector - Origin;

    return std::sqrt(Delta.x() * Delta.x() + Delta.y() * Delta.y());
}

void DiffusionCurveRenderer::OrthographicCamera::Reset()
{
    mZoom = 1.0f;
    mTop = 0.0f;
    mLeft = 0.0f;
}

QVector2D DiffusionCurveRenderer::OrthographicCamera::WorldToCamera(float X, float Y)
{
    X -= mLeft;
    Y -= mTop;

    return QVector2D(X / mZoom, Y / mZoom) / mPixelRatio;
}
