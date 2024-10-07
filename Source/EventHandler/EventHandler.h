#pragma once

#include "Core/EventListener.h"
#include "Curve/Spline.h"
#include "Structs/Enums.h"
#include "Structs/Mouse.h"
#include "Util/Macros.h"

namespace DiffusionCurveRenderer
{
    class RendererManager;
    class OrthographicCamera;
    class CurveContainer;

    class EventHandler : public QObject, public EventListener
    {
        Q_OBJECT
      public:
        explicit EventHandler(QObject* parent);

        void OnKeyPressed(QKeyEvent*) override;
        void OnKeyReleased(QKeyEvent*) override;
        void OnMousePressed(QMouseEvent*) override;
        void OnMouseReleased(QMouseEvent*) override;
        void OnMouseMoved(QMouseEvent*) override;
        void OnWheelMoved(QWheelEvent*) override;

        CurvePtr GetSelectedCurve() const { return mSelectedCurve; }

        void SetSelectedCurve(CurvePtr selectedCurve);
        void SetSelectedControlPoint(ControlPointPtr point);
        void SetSelectedColorPoint(ColorPointPtr point);

        void SetControlPointAround(ControlPointPtr point);
        void SetColorPointAround(ColorPointPtr point);

      signals:
        void SelectedCurveChanged(CurvePtr selectedCurve);
        void ControlPointAroundChanged(ControlPointPtr point);
        void ColorPointAroundChanged(ColorPointPtr point);
        void SelectedControlPointChanged(ControlPointPtr point);
        void SelectedColorPointChanged(ColorPointPtr point);

      private:
        CurvePtr GetCurveAround(float x, float y);
        ControlPointPtr GetControlPointAround(float x, float y);
        ColorPointPtr GetColorPointAround(float x, float y);
        QVector2D CameraToWorld(float x, float y);
        float CameraDistanceToWorldDistance(float distance);

        CurvePtr mSelectedCurve{ nullptr };
        ControlPointPtr mControlPointAround{ nullptr };
        ColorPointPtr mColorPointAround{ nullptr };
        ControlPointPtr mSelectedControlPoint{ nullptr };
        ColorPointPtr mSelectedColorPoint{ nullptr };

        Qt::Key mPressedKey{ Qt::Key_No };

        Mouse mMouse;

        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(RendererManager, RendererManager);
    };

}
