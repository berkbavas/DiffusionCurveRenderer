#pragma once

#include <QInputEvent>
#include <QOpenGLExtraFunctions>
#include <QOpenGLWindow>

namespace DiffusionCurveRenderer
{
    class Window : public QOpenGLWindow, public QOpenGLExtraFunctions
    {
        Q_OBJECT
      public:
        Window(QWindow* pParent = nullptr);

      private:
        void initializeGL() override;
        void resizeGL(int Width, int Height) override;
        void paintGL() override;
        void keyPressEvent(QKeyEvent* pEvent) override;
        void keyReleaseEvent(QKeyEvent* pEvent) override;
        void mousePressEvent(QMouseEvent* pEvent) override;
        void mouseReleaseEvent(QMouseEvent* pEvent) override;
        void mouseMoveEvent(QMouseEvent* pEvent) override;
        void wheelEvent(QWheelEvent* pEvent) override;

      signals:
        // Core Events
        void Initialize();
        void Resize(int Width, int Height);
        void Render(float Ifps);

        // Input Events
        void KeyPressed(QKeyEvent* pEvent);
        void KeyReleased(QKeyEvent* pEvent);
        void MousePressed(QMouseEvent* pEvent);
        void MouseReleased(QMouseEvent* pEvent);
        void MouseMoved(QMouseEvent* pEvent);
        void WheelMoved(QWheelEvent* pEvent);

      private:
        long long mPreviousTime;
        long long mCurrentTime;
    };
}
