#pragma once

#include "Core/Constants.h"
#include "Core/OrthographicCamera.h"
#include "Curve/Curve.h"
#include "Curve/Spline.h"
#include "Util/Macros.h"

#include <QObject>
#include <QPainter>
#include <QQueue>
#include <functional>

namespace DiffusionCurveRenderer
{
    class OverlayPainter : public QObject
    {
        Q_OBJECT
      public:
        explicit OverlayPainter(QObject* parent);

        void Render();

      private:
        void DrawControlPolygon();
        void PaintControlPointsHandles();
        void PaintColorPointsHandles();
        void PaintColorPointsHandles(BezierPtr bezier);

        void FillOuterDisk(QPainter& painter, QPointF center, float scaling, QBrush brush = QColor(128, 128, 128, 128));
        void FillInnerDisk(QPainter& painter, QPointF center, float scaling, QBrush brush = QColor(255, 255, 255));

        float GetZoomMultiplier() const;

        QPointF WorldToCamera(const QVector2D& world);
        QPointF GetColorPointHandlePosition(CurvePtr curve, ColorPointPtr colorPoint, bool useOffset);

        QPen mDashedPen;
        QPen mDenseDashedPen;
        QPen mSolidPen{ QColor(255, 255, 255) };
        QPen mClearPen{ QColor(0, 0, 0, 0) };

        DEFINE_MEMBER(CurvePtr, SelectedCurve);
        DEFINE_MEMBER_PTR(QPaintDevice, Device);
        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER(ControlPointPtr, SelectedControlPoint);
        DEFINE_MEMBER(ControlPointPtr, ControlPointAround);
        DEFINE_MEMBER(ColorPointPtr, SelectedColorPoint);
        DEFINE_MEMBER(ColorPointPtr, ColorPointAround);
        DEFINE_MEMBER(BlurPointPtr, SelectedBlurPoint);
        DEFINE_MEMBER(bool, PaintColorPointHandles, true);

        static constexpr float INNER_DISK_RADIUS{ HANDLE_INNER_DISK_RADIUS_PX };
        static constexpr float OUTER_DISK_RADIUS{ HANDLE_OUTER_DISK_RADIUS_PX };
        static constexpr float HANDLE_OFFSET{ COLOR_POINT_HANDLE_OFFSET_PX };
    };

}