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
        explicit EventHandler(QObject* pParent);

        void OnKeyPressed(QKeyEvent* pEvent) override;
        void OnKeyReleased(QKeyEvent* pEvent) override;
        void OnMousePressed(QMouseEvent* pEvent) override;
        void OnMouseReleased(QMouseEvent* pEvent) override;
        void OnMouseMoved(QMouseEvent* pEvent) override;
        void OnWheelMoved(QWheelEvent* pEvent) override;

        CurvePtr GetSelectedCurve() const { return mSelectedCurve; }
        void SetSelectedCurve(CurvePtr pSelectedCurve);
        void SetSelectedControlPoint(ControlPointPtr pPoint);
        void SetSelectedColorPoint(ColorPointPtr pPoint);
        void SetControlPointAround(ControlPointPtr pPoint);
        void SetColorPointAround(ColorPointPtr pPoint);

      signals:
        void SelectedCurveChanged(CurvePtr pSelectedCurve);
        void ControlPointAroundChanged(ControlPointPtr pPoint);
        void ColorPointAroundChanged(ColorPointPtr pPoint);
        void SelectedControlPointChanged(ControlPointPtr pPoint);
        void SelectedColorPointChanged(ColorPointPtr pPoint);

        // New signals for keyboard shortcuts
        void DuplicateCurveRequested();
        void ZoomToFitRequested();
        void ResetViewRequested();

      private:
        CurvePtr GetCurveAround(float X, float Y);
        ControlPointPtr GetControlPointAround(float X, float Y);
        ColorPointPtr GetColorPointAround(float X, float Y);
        QVector2D CameraToWorld(float X, float Y);
        float CameraDistanceToWorldDistance(float Distance);

        CurvePtr mSelectedCurve{ nullptr };
        ControlPointPtr mControlPointAround{ nullptr };
        ColorPointPtr mColorPointAround{ nullptr };
        ControlPointPtr mSelectedControlPoint{ nullptr };
        ColorPointPtr mSelectedColorPoint{ nullptr };

        Qt::Key mPressedKey{ Qt::Key_No };
        Qt::KeyboardModifiers mModifiers{ Qt::NoModifier };

        Mouse mMouse;

        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(RendererManager, RendererManager);
    };

}
