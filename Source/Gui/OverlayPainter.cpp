#include "OverlayPainter.h"

#include <QOpenGLPaintDevice>
#include <cmath>

DiffusionCurveRenderer::OverlayPainter::OverlayPainter(QObject* parent)
    : QObject(parent)
{
    mSolidPen.setWidthF(1.0f);

    mDashedPen.setDashPattern({ 8, 8 });
    mDashedPen.setWidthF(1.0f);
    mDashedPen.setJoinStyle(Qt::MiterJoin);
    mDashedPen.setColor(QColor(0, 0, 0));

    mDenseDashedPen.setDashPattern({ 4, 4 });
    mDenseDashedPen.setWidthF(1.0f);
    mDenseDashedPen.setJoinStyle(Qt::MiterJoin);
}

void DiffusionCurveRenderer::OverlayPainter::Render()
{
    if (mSelectedCurve == nullptr)
        return;

    DrawControlPolygon();
    PaintControlPointsHandles();

    if (mPaintColorPointHandles)
    {
        PaintColorPointsHandles();
    }
}

void DiffusionCurveRenderer::OverlayPainter::DrawControlPolygon()
{
    if (std::dynamic_pointer_cast<Spline>(mSelectedCurve)) // Don't draw control polygon for B-splines
        return;

    QPainter painter(mDevice);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(mDashedPen);
    painter.setBrush(QBrush());

    const auto& points = mSelectedCurve->GetControlPoints();

    for (int i = 0; i < points.size() - 1; ++i)
    {
        QPointF p0 = WorldToCamera(points[i]->position);
        QPointF p1 = WorldToCamera(points[i + 1]->position);
        painter.drawLine(p0, p1);
    }
}

void DiffusionCurveRenderer::OverlayPainter::PaintColorPointsHandles()
{
    if (const auto bezier = std::dynamic_pointer_cast<Bezier>(mSelectedCurve))
    {
        PaintColorPointsHandles(bezier);
    }
    else if (const auto spline = std::dynamic_pointer_cast<Spline>(mSelectedCurve))
    {
        const auto& patches = spline->GetBezierPatches();

        for (const auto& bezier : patches)
        {
            PaintColorPointsHandles(bezier);
        }
    }
}

void DiffusionCurveRenderer::OverlayPainter::PaintColorPointsHandles(BezierPtr bezier)
{
    QPainter painter(mDevice);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const auto& colorPoints = bezier->GetColorPoints();

    for (const auto colorPoint : colorPoints)
    {
        QPointF offset = GetColorPointHandlePosition(bezier, colorPoint, true);
        QPointF inset = GetColorPointHandlePosition(bezier, colorPoint, false);

        // Draw a dashed line actual position to visual position
        painter.setPen(mDenseDashedPen);
        painter.drawLine(inset, offset);

        // Outer disk
        float scaling = colorPoint == mColorPointAround ? 1.5 : 1.0f;
        FillOuterDisk(painter, offset, scaling);

        // Inner disk
        FillInnerDisk(painter, offset, 1.0f,
                      QColor(255 * colorPoint->color.x(),
                             255 * colorPoint->color.y(),
                             255 * colorPoint->color.z(),
                             255 * colorPoint->color.w()));
    }
}

void DiffusionCurveRenderer::OverlayPainter::FillOuterDisk(QPainter& painter, QPointF center, float scaling, QBrush brush)
{
    const float radius = scaling * OUTER_DISK_RADIUS;
    painter.setPen(mSolidPen);
    painter.setBrush(brush);
    painter.drawEllipse(center, radius, radius);
}

void DiffusionCurveRenderer::OverlayPainter::FillInnerDisk(QPainter& painter, QPointF center, float scaling, QBrush brush)
{
    const float radius = scaling * INNER_DISK_RADIUS;
    painter.setPen(mClearPen);
    painter.setBrush(brush);
    painter.drawEllipse(center, radius, radius);
}

void DiffusionCurveRenderer::OverlayPainter::PaintControlPointsHandles()
{
    const auto& points = mSelectedCurve->GetControlPoints();

    if (points.empty())
        return;

    QPainter painter(mDevice);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(mSolidPen);
    QPointF center;

    // Last control point
    center = WorldToCamera(points.last()->position);
    FillOuterDisk(painter, center, 1.0f);
    FillInnerDisk(painter, center, 1.0f, QColor(0, 255, 0));

    // Other control points
    for (int i = 0; i < points.size() - 1; ++i)
    {
        if (mControlPointAround == points[i])
            continue;

        center = WorldToCamera(points[i]->position);
        FillOuterDisk(painter, center, 1.0f);
        FillInnerDisk(painter, center, 1.0f);
    }

    // if (mSelectedControlPoint)
    // {
    //     center = WorldToCamera(mSelectedControlPoint->position);

    //     if (mSelectedControlPoint == points.first())
    //     {
    //         FillOuterDisk(painter, center, 2.0f);
    //         FillInnerDisk(painter, center, 2.0f, QColor(255, 255, 0));
    //     }
    //     else if (mSelectedControlPoint == points.last())
    //     {
    //         FillOuterDisk(painter, center, 2.0f);
    //         FillInnerDisk(painter, center, 2.0f, QColor(0, 255, 0));
    //     }
    //     else
    //     {
    //         FillOuterDisk(painter, center, 2.0f);
    //         FillInnerDisk(painter, center, 2.0f);
    //     }
    // }

    if (mControlPointAround)
    {
        center = WorldToCamera(mControlPointAround->position);

        if (mControlPointAround == points.last())
        {
            FillOuterDisk(painter, center, 1.5f);
            FillInnerDisk(painter, center, 1.5f, QColor(0, 255, 0));
        }
        else
        {
            FillOuterDisk(painter, center, 1.5f);
            FillInnerDisk(painter, center, 1.5f);
        }
    }
}

float DiffusionCurveRenderer::OverlayPainter::GetZoomMultiplier() const
{
    return 1.0f / std::pow(mCamera->GetZoom(), 0.5f);
}

QPointF DiffusionCurveRenderer::OverlayPainter::WorldToCamera(const QVector2D& world)
{
    return mCamera->WorldToCamera(world.toPointF());
}

QPointF DiffusionCurveRenderer::OverlayPainter::GetColorPointHandlePosition(CurvePtr curve, ColorPointPtr colorPoint, bool useOffset)
{
    float position = colorPoint->position;
    float offset = colorPoint->type == ColorPointType::Left ? HANDLE_OFFSET : -HANDLE_OFFSET;
    offset = mCamera->CameraDistanceToWorldDistance(offset);
    QVector2D worldPosition = curve->PositionAt(position) + useOffset * offset * curve->NormalAt(position);
    return WorldToCamera(worldPosition);
}
