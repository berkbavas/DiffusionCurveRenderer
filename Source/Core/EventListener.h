#pragma once

#include <QMouseEvent>

namespace DiffusionCurveRenderer
{
    class EventListener
    {
      public:
        EventListener() = default;

        virtual void OnKeyPressed(QKeyEvent*) {}
        virtual void OnKeyReleased(QKeyEvent*) {}
        virtual void OnMousePressed(QMouseEvent*) {}
        virtual void OnMouseReleased(QMouseEvent*) {}
        virtual void OnMouseMoved(QMouseEvent*) {}
        virtual void OnWheelMoved(QWheelEvent*) {}
    };
}