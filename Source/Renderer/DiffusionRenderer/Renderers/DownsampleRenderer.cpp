#include "DownsampleRenderer.h"

#include "Core/Constants.h"
#include "Util/Chronometer.h"

DiffusionCurveRenderer::DownsampleRenderer::DownsampleRenderer()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;

    mDownsampleShader = new Shader("Downsample Shader");
    mDownsampleShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mDownsampleShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Downsample.frag");
    mDownsampleShader->Initialize();

    mFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFramebufferFormat.setSamples(0);
    mFramebufferFormat.setMipmap(false);
    mFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::DownsampleRenderer::Downsample(QOpenGLFramebufferObject* pSource)
{
    MEASURE_CALL_TIME(DOWNSAMPLE_RENDERER);

    BlitSourceFramebuffer(pSource);

    for (int i = 1; i < mFramebuffers.size(); ++i)
    {
        Downsample(mFramebuffers[i - 1], mFramebuffers[i]);
    }
}

void DiffusionCurveRenderer::DownsampleRenderer::BlitSourceFramebuffer(QOpenGLFramebufferObject* pSource)
{
    QOpenGLFramebufferObject::blitFramebuffer(mFramebuffers[0], //
                                              QRect(0, 0, mFramebuffers[0]->width(), mFramebuffers[0]->height()),
                                              pSource,
                                              QRect(0, 0, pSource->width(), pSource->height()),
                                              GL_COLOR_BUFFER_BIT,
                                              GL_LINEAR,
                                              0,
                                              0);
}

void DiffusionCurveRenderer::DownsampleRenderer::Downsample(QOpenGLFramebufferObject* pSource, QOpenGLFramebufferObject* pTarget)
{
    pTarget->bind();
    glViewport(0, 0, pTarget->width(), pTarget->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mDownsampleShader->Bind();
    mDownsampleShader->SetSampler("uColorTexture", 0, pSource->textures().at(0));
    mQuad->Render();
    mDownsampleShader->Release();
    pTarget->release();
}

void DiffusionCurveRenderer::DownsampleRenderer::SetFramebufferSize(int Size)
{
    for (int I = 0; I < mFramebuffers.size(); ++I)
    {
        delete mFramebuffers[I];
    }

    mFramebuffers.clear();

    while (Size > 2)
    {
        mFramebuffers << new QOpenGLFramebufferObject(Size, Size, mFramebufferFormat);
        Size /= 2;
    }
}
