#ifndef POINTRENDERER_H
#define POINTRENDERER_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QVector2D>

#include <Renderers/Base/Ticks.h>

class PointRenderer : protected QOpenGLFunctions
{
public:
    PointRenderer();
    ~PointRenderer();

    struct Parameters
    {
        QVector2D point;
        float innerRadius;
        float outerRadius;
        QVector4D innerColor;
        QVector4D outerColor;
    };

    bool init();
    void render(const Parameters &parameters, const QMatrix4x4 &projectionMatrix);

private:
    QOpenGLShaderProgram *mShader;
    Ticks *mTicks;

    int mProjectionMatrixLocation;
    int mPointLocation;
    int mInnerColorLocation;
    int mOuterColorLocation;
    int mInnerRadiusLocation;
    int mOuterRadiusLocation;
    int mTicksDeltaLocation;
};

#endif // POINTRENDERER_H
