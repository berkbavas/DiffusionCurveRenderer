#include "UpsampleRenderer.h"

#include "Core/Constants.h"
#include "Util/Chronometer.h"

#include <QImage>

DiffusionCurveRenderer::UpsampleRenderer::UpsampleRenderer()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;

    mUpsampleShader = new Shader("Upsample Shader");
    mUpsampleShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mUpsampleShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Upsample.frag");
    mUpsampleShader->Initialize();

    mJacobiShader = new Shader("Jacobi Shader");
    mJacobiShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mJacobiShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Jacobi.frag");
    mJacobiShader->Initialize();

    mFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFramebufferFormat.setSamples(0);
    mFramebufferFormat.setMipmap(false);
    mFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::UpsampleRenderer::Upsample(const QVector<QOpenGLFramebufferObject*>& Downsamples)
{
    Chronometer Chronometer("UpsampleRenderer::Upsample");

    BlitSourceFramebuffer(Downsamples.last());

    for (int i = mUpsampleFramebuffers.size() - 2; i >= 0; --i)
    {
        Upsample(mUpsampleFramebuffers[i], mTemporaryFramebuffers[i], mUpsampleFramebuffers[i + 1], Downsamples[i]);
    }
}

void DiffusionCurveRenderer::UpsampleRenderer::Upsample(QOpenGLFramebufferObject* pTarget, QOpenGLFramebufferObject* pTemporary, QOpenGLFramebufferObject* pSource, QOpenGLFramebufferObject* pConstraint)
{
    pTarget->bind();
    glViewport(0, 0, pTarget->width(), pTarget->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mUpsampleShader->Bind();
    mUpsampleShader->SetSampler("uColorSourceTexture", 0, pSource->textures().at(0));
    mUpsampleShader->SetSampler("uColorTargetTexture", 1, pConstraint->textures().at(0));
    mQuad->Render();
    mUpsampleShader->Release();

    for (int j = 0; j < mSmoothIterations; j++)
    {
        if (j % 2 == 0)
        {
            pTemporary->bind();
            glViewport(0, 0, pTemporary->width(), pTemporary->height());

            mJacobiShader->Bind();
            mJacobiShader->SetSampler("uColorConstrainedTexture", 0, pConstraint->textures().at(0));
            mJacobiShader->SetSampler("uColorTargetTexture", 1, pTarget->textures().at(0));
            mQuad->Render();
            mJacobiShader->Release();
            pTemporary->release();
        }

        else
        {
            pTarget->bind();
            glViewport(0, 0, pTarget->width(), pTarget->height());

            mJacobiShader->Bind();
            mJacobiShader->SetSampler("uColorConstrainedTexture", 0, pConstraint->textures().at(0));
            mJacobiShader->SetSampler("uColorTargetTexture", 1, pTemporary->textures().at(0));
            mQuad->Render();
            mJacobiShader->Release();
            pTarget->release();
        }
    }
}

void DiffusionCurveRenderer::UpsampleRenderer::SetFramebufferSize(int Size)
{
    for (int I = 0; I < mUpsampleFramebuffers.size(); ++I)
    {
        delete mUpsampleFramebuffers[I];
    }

    for (int I = 0; I < mTemporaryFramebuffers.size(); ++I)
    {
        delete mTemporaryFramebuffers[I];
    }

    mUpsampleFramebuffers.clear();
    mTemporaryFramebuffers.clear();

    while (Size > 2)
    {
        mUpsampleFramebuffers << new QOpenGLFramebufferObject(Size, Size, mFramebufferFormat);

        mTemporaryFramebuffers << new QOpenGLFramebufferObject(Size, Size, mFramebufferFormat);

        Size /= 2;
    }
}

void DiffusionCurveRenderer::UpsampleRenderer::BlitSourceFramebuffer(QOpenGLFramebufferObject* pSource)
{
    QOpenGLFramebufferObject::blitFramebuffer(mUpsampleFramebuffers.last(), //
                                              QRect(0, 0, mUpsampleFramebuffers.last()->width(), mUpsampleFramebuffers.last()->height()),
                                              pSource,
                                              QRect(0, 0, pSource->width(), pSource->height()),
                                              GL_COLOR_BUFFER_BIT,
                                              GL_LINEAR,
                                              0,
                                              0);
}
