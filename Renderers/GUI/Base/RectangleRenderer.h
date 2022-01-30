#ifndef RECTANGLERENDERER_H
#define RECTANGLERENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QVector4D>
#include <QVector>

#include <Renderers/Base/Ticks.h>

class RectangleRenderer : protected QOpenGLFunctions
{
public:
    RectangleRenderer();
    ~RectangleRenderer();

    struct Parameters
    {
        QRectF rectangle;
        QVector4D fillColor;
        QVector4D borderColor;
        bool borderEnabled;
        float borderWidth;
    };

    bool init();
    void render(const Parameters &parameters, const QMatrix4x4 &projectionMatrix);

private:
    QOpenGLShaderProgram *mShader;
    Ticks *mTicks;

    int mProjectionMatrixLocation;
    int mFillColorLocation;
    int mBorderEnabledLocation;
    int mBorderWidthLocation;
    int mBorderColorLocation;
    int mWidthLocation;
    int mHeightLocation;
    int mRectangleTopLeftLocation;
};

#endif // RECTANGLERENDERER_H
