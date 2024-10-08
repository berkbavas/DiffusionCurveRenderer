#include "Blitter.h"

DiffusionCurveRenderer::Blitter::Blitter()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;

    mScreenShader = new Shader("Screen Shader");
    mScreenShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mScreenShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Screen.frag");
    mScreenShader->Initialize();
}

void DiffusionCurveRenderer::Blitter::Blit(QOpenGLFramebufferObject* target, QOpenGLFramebufferObject* source, bool clearTarget)
{
    if (target == nullptr)
    {
        QOpenGLFramebufferObject::bindDefault();
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        target->bind();
        glViewport(0, 0, target->width(), target->height());
    }

    if (clearTarget)
    {
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    mScreenShader->Bind();
    mScreenShader->SetSampler("sourceTexture", 0, source->texture());
    mQuad->Render();
    mScreenShader->Release();
}
