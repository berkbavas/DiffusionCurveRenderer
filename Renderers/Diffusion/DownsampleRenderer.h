#ifndef DOWNSAMPLERENDERER_H
#define DOWNSAMPLERENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <Renderers/Base/Quads.h>

class DownsampleRenderer : protected QOpenGLFunctions
{
public:
    DownsampleRenderer();
    ~DownsampleRenderer();

    bool init();
    void render(GLuint texture);

private:
    QOpenGLShaderProgram *mShader;
    Quads *mQuads;

    int mSourceTextureLocation;

    bool mInit;
};

#endif // DOWNSAMPLERENDERER_H
