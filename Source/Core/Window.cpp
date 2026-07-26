#include "Window.h"

#include "Util/Logger.h"

#include <QDateTime>
#include <QDebug>
#include <QKeyEvent>

DiffusionCurveRenderer::Window::Window(QWindow* parent)
    : QOpenGLWindow(QOpenGLWindow::UpdateBehavior::NoPartialUpdate, parent)

{
    QSurfaceFormat Format = QSurfaceFormat::defaultFormat();
    Format.setSamples(8);
    setFormat(Format);

    connect(this, &QOpenGLWindow::frameSwapped, [=]() { update(); });
}

void DiffusionCurveRenderer::Window::initializeGL()
{
    initializeOpenGLFunctions();

    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    mPreviousTime = mCurrentTime;

    emit Initialize();
}

void DiffusionCurveRenderer::Window::resizeGL(int Width, int Height)
{
    emit Resize(Width, Height);
}

void DiffusionCurveRenderer::Window::paintGL()
{
    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    const float Ifps = (mCurrentTime - mPreviousTime) * 0.001f;
    mPreviousTime = mCurrentTime;

    emit Render(Ifps);
}

void DiffusionCurveRenderer::Window::keyPressEvent(QKeyEvent* pEvent)
{
    emit KeyPressed(pEvent);
}

void DiffusionCurveRenderer::Window::keyReleaseEvent(QKeyEvent* pEvent)
{
    emit KeyReleased(pEvent);
}

void DiffusionCurveRenderer::Window::mousePressEvent(QMouseEvent* pEvent)
{
    emit MousePressed(pEvent);
}

void DiffusionCurveRenderer::Window::mouseReleaseEvent(QMouseEvent* pEvent)
{
    emit MouseReleased(pEvent);
}

void DiffusionCurveRenderer::Window::mouseMoveEvent(QMouseEvent* pEvent)
{
    emit MouseMoved(pEvent);
}

void DiffusionCurveRenderer::Window::wheelEvent(QWheelEvent* pEvent)
{
    emit WheelMoved(pEvent);
}
