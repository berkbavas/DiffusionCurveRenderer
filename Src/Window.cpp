#include "Window.h"
#include "Controller.h"

#include <QDateTime>
#include <QKeyEvent>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QPen>

#include <QDebug>

DiffusionCurveRenderer::Window::Window(QWindow *parent)
    : QOpenGLWindow(QOpenGLWindow::UpdateBehavior::NoPartialUpdate, parent)
    , mController(nullptr)

{
    connect(this, &QOpenGLWindow::frameSwapped, this, [=]() { update(); });
}

DiffusionCurveRenderer::Window::~Window()
{
    if (mController)
        mController->deleteLater();
}

void DiffusionCurveRenderer::Window::initializeGL()
{
    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    mPreviousTime = mCurrentTime;

    initializeOpenGLFunctions();

    QtImGui::initialize(this);
    mController = new Controller;
    mController->SetWindow(this);
    mController->Init();
}

void DiffusionCurveRenderer::Window::resizeGL(int w, int h)
{
    glViewport(0, 0, width(), height());

    mController->Resize(w, h);
}

void DiffusionCurveRenderer::Window::paintGL()
{
    mCurrentTime = QDateTime::currentMSecsSinceEpoch();
    float ifps = (mCurrentTime - mPreviousTime) * 0.001f;
    mPreviousTime = mCurrentTime;

    mController->Render(ifps);
}

void DiffusionCurveRenderer::Window::keyPressEvent(QKeyEvent *event)
{
    mController->OnKeyPressed(event);
}

void DiffusionCurveRenderer::Window::keyReleaseEvent(QKeyEvent *event)
{
    mController->OnKeyReleased(event);
}

void DiffusionCurveRenderer::Window::mousePressEvent(QMouseEvent *event)
{
    mController->OnMousePressed(event);
}

void DiffusionCurveRenderer::Window::mouseReleaseEvent(QMouseEvent *event)
{
    mController->OnMouseReleased(event);
}

void DiffusionCurveRenderer::Window::mouseMoveEvent(QMouseEvent *event)
{
    mController->OnMouseMoved(event);
}

void DiffusionCurveRenderer::Window::wheelEvent(QWheelEvent *event)
{
    mController->OnWheelMoved(event);
}
