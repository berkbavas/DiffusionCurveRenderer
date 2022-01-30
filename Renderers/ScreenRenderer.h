#ifndef SCREENRENDERER_H
#define SCREENRENDERER_H

#include <Renderers/Base/Quads.h>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

class ScreenRenderer : protected QOpenGLFunctions
{
public:
    ScreenRenderer();
    ~ScreenRenderer();

    bool init();
    void render(GLuint texture, float pixelRatio);

private:
    QOpenGLShaderProgram *mShader;
    Quads *mQuads;

    int mTextureLocation;
    int mPixelRatioLocation;
};

#endif // SCREENRENDERER_H
