#include "Transformer.h"

Transformer::Transformer() {}

QVector2D Transformer::mapFromGuiToOpenGL(const QPointF &position) const
{
    float x = mProjectionParameters->left + mProjectionParameters->zoomRatio * position.x();
    float y = mProjectionParameters->top + mProjectionParameters->zoomRatio * position.y();

    return QVector2D(x, y);
}

QPointF Transformer::mapFromOpenGLToGui(const QVector2D &position) const
{
    const float &zoomRatio = mProjectionParameters->zoomRatio;
    const float &pixelRatio = mProjectionParameters->pixelRatio;

    float x = position.x() - mProjectionParameters->left;
    float y = position.y() - mProjectionParameters->top;

    return QPointF(x / zoomRatio * pixelRatio, y / zoomRatio * pixelRatio);
}

QRectF Transformer::mapFromOpenGLToGui(const QRectF &rect) const
{
    const float &zoomRatio = mProjectionParameters->zoomRatio;
    const float &pixelRatio = mProjectionParameters->pixelRatio;

    float w = rect.width() / zoomRatio * pixelRatio;
    float h = rect.height() / zoomRatio * pixelRatio;

    QPointF center = mapFromOpenGLToGui(QVector2D(rect.center().x(), rect.center().y()));

    return QRectF(center.x() - 0.5 * w, center.y() - 0.5 * h, w, h);
}

void Transformer::setProjectionParameters(const ProjectionParameters *newProjectionParameters)
{
    mProjectionParameters = newProjectionParameters;
}
