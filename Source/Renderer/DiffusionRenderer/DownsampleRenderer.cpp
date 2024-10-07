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

    CreateFramebuffers();
}

void DiffusionCurveRenderer::DownsampleRenderer::Downsample(QOpenGLFramebufferObject* source)
{
    MEASURE_CALL_TIME(DOWNSAMPLE_RENDERER);

    BlitSourceFramebuffer(source);

    for (int i = 1; i < mFramebuffers.size(); ++i)
    {
        Downsample(mFramebuffers[i - 1], mFramebuffers[i]);
    }
}

void DiffusionCurveRenderer::DownsampleRenderer::BlitSourceFramebuffer(QOpenGLFramebufferObject* source)
{
    for (int attachment = 0; attachment < 2; attachment++)
    {
        QOpenGLFramebufferObject::blitFramebuffer(
            mFramebuffers[0],
            QRect(0, 0, mFramebuffers[0]->width(), mFramebuffers[0]->height()),
            source,
            QRect(0, 0, source->width(), source->height()),
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR,
            attachment,
            attachment);
    }
}

void DiffusionCurveRenderer::DownsampleRenderer::Downsample(QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* target)
{
    target->bind();
    glViewport(0, 0, target->width(), target->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mDownsampleShader->Bind();
    mDownsampleShader->SetSampler("colorTexture", 0, source->textures().at(0));
    mDownsampleShader->SetSampler("blurTexture", 1, source->textures().at(1));
    mQuad->Render();
    mDownsampleShader->Release();
    target->release();
}

void DiffusionCurveRenderer::DownsampleRenderer::DeleteFramebuffers()
{
    for (int i = 0; i < mFramebuffers.size(); ++i)
    {
        delete mFramebuffers[i];
    }

    mFramebuffers.clear();
}

void DiffusionCurveRenderer::DownsampleRenderer::CreateFramebuffers()
{
    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    int size = mFramebufferSize;

    while (size > 2)
    {
        mFramebuffers << new QOpenGLFramebufferObject(size, size, mFramebufferFormat);
        mFramebuffers.last()->addColorAttachment(size, size); // For blur
        mFramebuffers.last()->bind();
        glDrawBuffers(2, ATTACHMENTS);
        mFramebuffers.last()->release();

        size /= 2;
    }
}
