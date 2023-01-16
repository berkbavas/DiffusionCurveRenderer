#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctionsPrivate>
#include <QRandomGenerator>
#include <QWheelEvent>

#include <imgui.h>
#include <QPen>
#include <QtImGui.h>

#include <QOpenGLWindow>

namespace DiffusionCurveRenderer
{
    class Controller;

    class Window : public QOpenGLWindow, protected QOpenGLExtraFunctions
    {
        Q_OBJECT

    public:
        Window(QWindow* parent = nullptr);
        ~Window();

    private:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;
        void keyPressEvent(QKeyEvent*) override;
        void keyReleaseEvent(QKeyEvent*) override;
        void mousePressEvent(QMouseEvent*) override;
        void mouseReleaseEvent(QMouseEvent*) override;
        void mouseMoveEvent(QMouseEvent*) override;
        void wheelEvent(QWheelEvent*) override;

    private:
        Controller* mController;
        long long mPreviousTime;
        long long mCurrentTime;
    };
}
