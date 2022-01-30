#ifndef COLORRENDERER_H
#define COLORRENDERER_H

#include <Types.h>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <Renderers/Base/Ticks.h>

#include <Curves/Curve.h>

class ColorRenderer : protected QOpenGLFunctions
{
public:
    struct Parameters
    {
        ColorRendererMode mode;
        QVector<Curve *> curves;
        QMatrix4x4 projectionMatrix;
        float gap;
        float width;
        int quality;
    };

    ColorRenderer();
    ~ColorRenderer();

    bool init();
    void render(const Parameters &parameters);

private:
    QOpenGLShaderProgram *mLineStripShader;
    QMap<QString, Location> mLineStripLocations;

    QOpenGLShaderProgram *mTriangleStripShader;
    QMap<QString, Location> mTriangleStripLocations;

    Ticks *mTicks;
    bool mInit;
};

#endif // COLORRENDERER_H
