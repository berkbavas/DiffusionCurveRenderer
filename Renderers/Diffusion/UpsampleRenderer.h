#ifndef UPSAMPLERENDERER_H
#define UPSAMPLERENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <Renderers/Base/Quads.h>

class UpsampleRenderer : protected QOpenGLFunctions
{
public:
    UpsampleRenderer();
    ~UpsampleRenderer();

    bool init();
    void render(GLuint sourceTexture, GLuint targetTexture);

private:
    QOpenGLShaderProgram *mShader;
    Quads *mQuads;

    int mSourceTextureLocation;
    int mTargetTextureLocation;
    bool mInit;
};

#endif // UPSAMPLERENDERER_H
