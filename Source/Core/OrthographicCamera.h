#pragma once

#include "Core/Constants.h"
#include "Core/EventListener.h"
#include "Structs/Mouse.h"
#include "Util/Macros.h"

#include <QMatrix4x4>
#include <QObject>

namespace DiffusionCurveRenderer
{
    class OrthographicCamera : public EventListener
    {
      public:
        OrthographicCamera() = default;

        void Resize(int Width, int Height, float PixelRatio);

        void OnMousePressed(QMouseEvent* pEvent) override;
        void OnMouseReleased(QMouseEvent* pEvent) override;
        void OnMouseMoved(QMouseEvent* pEvent) override;
        void OnWheelMoved(QWheelEvent* pEvent) override;

        const QMatrix4x4& GetProjectionMatrix();

        float GetAspectRatio() const;

        QVector2D CameraToWorld(float X, float Y);
        QVector2D CameraToWorld(const QVector2D& Camera);
        QPointF CameraToWorld(const QPointF& Camera);

        QVector2D WorldToCamera(float X, float Y);
        QVector2D WorldToCamera(const QVector2D& World);
        QPointF WorldToCamera(const QPointF& World);

        float WorldDistanceToCameraDistance(float Distance);
        float CameraDistanceToWorldDistance(float Distance);

        void Reset();

      private:
        DEFINE_MEMBER(float, PixelRatio, 1.0f);
        DEFINE_MEMBER(int, Width, INITIAL_WIDTH);
        DEFINE_MEMBER(int, Height, INITIAL_HEIGHT);
        DEFINE_MEMBER(float, Zoom, 1.0f);
        DEFINE_MEMBER(float, Top, 0.0f);
        DEFINE_MEMBER(float, Left, 0.0f);
        DEFINE_MEMBER(float, ZNear, -1.0f);
        DEFINE_MEMBER(float, ZFar, 1.0f);
        DEFINE_MEMBER(Qt::MouseButton, ActionReceiveButton, Qt::MiddleButton)

        Mouse mMouse;
        bool mUpdatePosition{ false };

        float mTimeElapsed{ 0 };

        QMatrix4x4 mProjectionMatrix;
    };
}
