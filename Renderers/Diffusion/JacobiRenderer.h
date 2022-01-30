#ifndef JACOBIRENDERER_H
#define JACOBIRENDERER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <Renderers/Base/Quads.h>

class JacobiRenderer : protected QOpenGLFunctions
{
public:
    JacobiRenderer();
    ~JacobiRenderer();

    bool init();
    void render(GLuint constrainedTexture, GLuint targetTexture);

private:
    QOpenGLShaderProgram *mShader;
    Quads *mQuads;

    int mConstrainedTextureLocation;
    int mTargetTextureLocation;

    bool mInit;
};

#endif // JACOBIRENDERER_H
