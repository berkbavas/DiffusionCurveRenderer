#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "Types.h"

#include <QPointF>
#include <QVector2D>

class Transformer
{
public:
    Transformer();

    QVector2D mapFromGuiToOpenGL(const QPointF &position) const;
    QPointF mapFromOpenGLToGui(const QVector2D &position) const;
    QRectF mapFromOpenGLToGui(const QRectF &rect) const;

    void setProjectionParameters(const ProjectionParameters *newProjectionParameters);

private:
    const ProjectionParameters *mProjectionParameters;
};

#endif // TRANSFORMER_H
