#include "Window.h"

#include "Util/Logger.h"

#include <QDateTime>
#include <QDebug>
#include <QKeyEvent>

DiffusionCurveRenderer::Window::Window(QWindow* parent)
    : QOpenGLWindow(QOpenGLWindow::UpdateBehavior::NoPartialUpdate, parent)

{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(8);
    setFormat(format);

    connect(this, &QOpenGLWindow::frameSwapped, [=]()
            { update(); });
}

void DiffusionCurveRenderer::Window::initializeGL()
{
    initializeOpenGLFunctions();

    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    mPreviousTime = mCurrentTime;

    emit Initialize();
}

void DiffusionCurveRenderer::Window::resizeGL(int width, int height)
{
    emit Resize(width, height);
}

void DiffusionCurveRenderer::Window::paintGL()
{
    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    const float ifps = (mCurrentTime - mPreviousTime) * 0.001f;
    mPreviousTime = mCurrentTime;

    emit Render(ifps);
}

void DiffusionCurveRenderer::Window::keyPressEvent(QKeyEvent* event)
{
    emit KeyPressed(event);
}

void DiffusionCurveRenderer::Window::keyReleaseEvent(QKeyEvent* event)
{
    emit KeyReleased(event);
}

void DiffusionCurveRenderer::Window::mousePressEvent(QMouseEvent* event)
{
    emit MousePressed(event);
}

void DiffusionCurveRenderer::Window::mouseReleaseEvent(QMouseEvent* event)
{
    emit MouseReleased(event);
}

void DiffusionCurveRenderer::Window::mouseMoveEvent(QMouseEvent* event)
{
    emit MouseMoved(event);
}

void DiffusionCurveRenderer::Window::wheelEvent(QWheelEvent* event)
{
    emit WheelMoved(event);
}
