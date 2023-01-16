#pragma once

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QObject>

namespace DiffusionCurveRenderer
{
    class EditModeCamera : public QObject
    {
        Q_OBJECT;
        explicit EditModeCamera(QObject* parent = nullptr);

    public:
        static EditModeCamera* Instance();

        void OnMousePressed(QMouseEvent* event);
        void OnMouseReleased(QMouseEvent* event);
        void OnMouseMoved(QMouseEvent* event);
        void OnWheelMoved(QWheelEvent* event);
        void Resize(int width, int height);
        void Update(float ifps);

        QMatrix4x4 GetProjection() const;

        QVector2D ToOpenGL(const QPointF& position) const;
        QPointF ToGUI(const QPointF& position) const;
        QPointF ToGUI(const QVector2D& position) const;
        QRectF ToGUI(const QRectF& rect) const;

        float GetPixelRatio() const;
        void SetPixelRatio(float newPixelRatio);

        float GetZoom() const;
        void SetZoom(float newZoom);

        float GetLeft() const;
        void SetLeft(float newLeft);

        float GetTop() const;
        void SetTop(float newTop);

        float GetWidth() const;
        float GetHeight() const;

    private:
        float mZNear;
        float mZFar;
        float mZoom;
        float mLeft;
        float mTop;
        float mWidth;
        float mHeight;
        float mPixelRatio;

        bool mUpdatePosition;

        struct Mouse {
            bool pressed = false;
            float x = 0;
            float y = 0;
            float dx = 0;
            float dy = 0;
        } mMouse;
    };
}
