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

        void Resize(int width, int height, float pixelRatio);

        void OnMousePressed(QMouseEvent* event) override;
        void OnMouseReleased(QMouseEvent* event) override;
        void OnMouseMoved(QMouseEvent* event) override;
        void OnWheelMoved(QWheelEvent* event) override;

        const QMatrix4x4& GetProjectionMatrix();

        float GetAspectRatio() const;

        QVector2D CameraToWorld(float x, float y);
        QVector2D CameraToWorld(const QVector2D& camera);
        QPointF CameraToWorld(const QPointF& camera);

        QVector2D WorldToCamera(float x, float y);
        QVector2D WorldToCamera(const QVector2D& world);
        QPointF WorldToCamera(const QPointF& world);

        float WorldDistanceToCameraDistance(float distance);
        float CameraDistanceToWorldDistance(float distance);

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
