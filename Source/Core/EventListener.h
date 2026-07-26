#pragma once

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace DiffusionCurveRenderer
{
    class EventListener
    {
      public:
        EventListener() = default;

        virtual void OnKeyPressed(QKeyEvent* pEvent);
        virtual void OnKeyReleased(QKeyEvent* pEvent);
        virtual void OnMousePressed(QMouseEvent* pEvent);
        virtual void OnMouseReleased(QMouseEvent* pEvent);
        virtual void OnMouseMoved(QMouseEvent* pEvent);
        virtual void OnWheelMoved(QWheelEvent* pEvent);
    };
}