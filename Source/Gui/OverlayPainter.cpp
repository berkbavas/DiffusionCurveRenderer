#include "OverlayPainter.h"

#include <cmath>

#include <QOpenGLPaintDevice>

DiffusionCurveRenderer::OverlayPainter::OverlayPainter(QObject* pParent)
    : QObject(pParent)
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
    // Draw grid first if enabled
    if (mShowGrid)
    {
        DrawGrid();
    }

    if (mSelectedCurve == nullptr)
    {
        return;
    }

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
    {
        return;
    }

    QPainter Painter(mDevice);
    Painter.setRenderHint(QPainter::Antialiasing, true);

    Painter.setPen(mDashedPen);
    Painter.setBrush(QBrush());

    const auto& Points = mSelectedCurve->GetControlPoints();

    for (int i = 0; i < Points.size() - 1; ++i)
    {
        const auto P0 = WorldToCamera(Points[i]->Position);
        const auto P1 = WorldToCamera(Points[i + 1]->Position);
        Painter.drawLine(P0, P1);
    }
}

void DiffusionCurveRenderer::OverlayPainter::PaintColorPointsHandles()
{
    if (const auto& pBezier = std::dynamic_pointer_cast<Bezier>(mSelectedCurve))
    {
        PaintColorPointsHandles(pBezier);
    }
    else if (const auto& pSpline = std::dynamic_pointer_cast<Spline>(mSelectedCurve))
    {
        const auto& Patches = pSpline->GetBezierPatches();

        for (const auto& Bezier : Patches)
        {
            PaintColorPointsHandles(Bezier);
        }
    }
}

void DiffusionCurveRenderer::OverlayPainter::PaintColorPointsHandles(BezierPtr pBezier)
{
    QPainter Painter(mDevice);
    Painter.setRenderHint(QPainter::Antialiasing, true);

    const auto& ColorPoints = pBezier->GetColorPoints();

    for (const auto& pColorPoint : ColorPoints)
    {
        const auto Offset = GetColorPointHandlePosition(pBezier, pColorPoint, true);
        const auto Inset = GetColorPointHandlePosition(pBezier, pColorPoint, false);

        // Draw a dashed line actual position to visual position
        Painter.setPen(mDenseDashedPen);
        Painter.drawLine(Inset, Offset);

        // Outer disk
        const auto Scaling = pColorPoint == mColorPointAround ? 1.5 : 1.0f;
        FillOuterDisk(Painter, Offset, Scaling);

        // Inner disk
        FillInnerDisk(Painter, Offset, 1.0f, QColor(255 * pColorPoint->Color.x(), 255 * pColorPoint->Color.y(), 255 * pColorPoint->Color.z(), 255 * pColorPoint->Color.w()));
    }
}

void DiffusionCurveRenderer::OverlayPainter::FillOuterDisk(QPainter& Painter, QPointF Center, float Scaling, QBrush Brush)
{
    const float Radius = Scaling * OUTER_DISK_RADIUS;
    Painter.setPen(mSolidPen);
    Painter.setBrush(Brush);
    Painter.drawEllipse(Center, Radius, Radius);
}

void DiffusionCurveRenderer::OverlayPainter::FillInnerDisk(QPainter& Painter, QPointF Center, float Scaling, QBrush Brush)
{
    const float Radius = Scaling * INNER_DISK_RADIUS;
    Painter.setPen(mClearPen);
    Painter.setBrush(Brush);
    Painter.drawEllipse(Center, Radius, Radius);
}

void DiffusionCurveRenderer::OverlayPainter::PaintControlPointsHandles()
{
    const auto& Points = mSelectedCurve->GetControlPoints();

    if (Points.empty())
    {
        return;
    }

    QPainter Painter(mDevice);
    Painter.setRenderHint(QPainter::Antialiasing, true);
    Painter.setPen(mSolidPen);
    QPointF Center;

    // Last control point
    Center = WorldToCamera(Points.last()->Position);
    FillOuterDisk(Painter, Center, 1.0f);
    FillInnerDisk(Painter, Center, 1.0f, QColor(0, 255, 0));

    // Other control points
    for (int I = 0; I < Points.size() - 1; ++I)
    {
        if (mControlPointAround == Points[I])
        {
            continue;
        }

        Center = WorldToCamera(Points[I]->Position);
        FillOuterDisk(Painter, Center, 1.0f);
        FillInnerDisk(Painter, Center, 1.0f);
    }

    if (mControlPointAround)
    {
        Center = WorldToCamera(mControlPointAround->Position);

        if (mControlPointAround == Points.last())
        {
            FillOuterDisk(Painter, Center, 1.5f);
            FillInnerDisk(Painter, Center, 1.5f, QColor(0, 255, 0));
        }
        else
        {
            FillOuterDisk(Painter, Center, 1.5f);
            FillInnerDisk(Painter, Center, 1.5f);
        }
    }
}

float DiffusionCurveRenderer::OverlayPainter::GetZoomMultiplier() const
{
    return 1.0f / std::pow(mCamera->GetZoom(), 0.5f);
}

QPointF DiffusionCurveRenderer::OverlayPainter::WorldToCamera(const QVector2D& World)
{
    return mCamera->WorldToCamera(World.toPointF());
}

QPointF DiffusionCurveRenderer::OverlayPainter::GetColorPointHandlePosition(CurvePtr pCurve, ColorPointPtr pColorPoint, bool UseOffset)
{
    const auto Position = pColorPoint->Position;
    auto Offset = pColorPoint->Type == ColorPointType::Left ? HANDLE_OFFSET : -HANDLE_OFFSET;
    Offset = mCamera->CameraDistanceToWorldDistance(Offset);
    const auto WorldPosition = pCurve->PositionAt(Position) + UseOffset * Offset * pCurve->NormalAt(Position);
    return WorldToCamera(WorldPosition);
}

void DiffusionCurveRenderer::OverlayPainter::DrawGrid()
{
    QPainter Painter(mDevice);
    Painter.setRenderHint(QPainter::Antialiasing, false);
    Painter.setPen(mGridPen);

    const auto DeviceWidth = mDevice->width();
    const auto DeviceHeight = mDevice->height();

    // Calculate world coordinates of viewport corners
    const auto TopLeft = mCamera->CameraToWorld(QVector2D(0, 0));
    const auto BottomRight = mCamera->CameraToWorld(QVector2D(DeviceWidth, DeviceHeight));

    // Adjust grid spacing based on zoom level
    float AdjustedSpacing = mGridSpacing;
    const auto Zoom = mCamera->GetZoom();

    // Scale grid to maintain reasonable density at different zoom levels
    while (AdjustedSpacing * Zoom < 20)
        AdjustedSpacing *= 2;
    while (AdjustedSpacing * Zoom > 100)
        AdjustedSpacing /= 2;

    // Calculate starting points (snap to grid)
    const auto StartX = std::floor(TopLeft.x() / AdjustedSpacing) * AdjustedSpacing;
    const auto StartY = std::floor(TopLeft.y() / AdjustedSpacing) * AdjustedSpacing;

    // Draw vertical lines
    for (float X = StartX; X <= BottomRight.x(); X += AdjustedSpacing)
    {
        QPointF P0 = mCamera->WorldToCamera(QPointF(X, TopLeft.y()));
        QPointF P1 = mCamera->WorldToCamera(QPointF(X, BottomRight.y()));
        Painter.drawLine(P0, P1);
    }

    // Draw horizontal lines
    for (float Y = StartY; Y <= BottomRight.y(); Y += AdjustedSpacing)
    {
        QPointF P0 = mCamera->WorldToCamera(QPointF(TopLeft.x(), Y));
        QPointF P1 = mCamera->WorldToCamera(QPointF(BottomRight.x(), Y));
        Painter.drawLine(P0, P1);
    }

    // Draw origin crosshair (thicker lines at origin)
    QPen OriginPen(QColor(150, 150, 150, 120));
    OriginPen.setWidth(2);
    Painter.setPen(OriginPen);

    // Vertical line at x=0
    if (TopLeft.x() <= 0 && BottomRight.x() >= 0)
    {
        QPointF P0 = mCamera->WorldToCamera(QPointF(0, TopLeft.y()));
        QPointF P1 = mCamera->WorldToCamera(QPointF(0, BottomRight.y()));
        Painter.drawLine(P0, P1);
    }

    // Horizontal line at y=0
    if (TopLeft.y() <= 0 && BottomRight.y() >= 0)
    {
        QPointF P0 = mCamera->WorldToCamera(QPointF(TopLeft.x(), 0));
        QPointF P1 = mCamera->WorldToCamera(QPointF(BottomRight.x(), 0));
        Painter.drawLine(P0, P1);
    }
}
