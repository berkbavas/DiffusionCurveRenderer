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

    CreateFramebuffers();
}

void DiffusionCurveRenderer::UpsampleRenderer::Upsample(QVector<QOpenGLFramebufferObject*> downsampleFramebuffers)
{
    MEASURE_CALL_TIME(UPSAMPLE_RENDERER);

    BlitSourceFramebuffer(downsampleFramebuffers.last());

    for (int i = mUpsampleFramebuffers.size() - 2; i >= 0; --i)
    {
        Upsample(mUpsampleFramebuffers[i], mTemporaryFramebuffers[i], mUpsampleFramebuffers[i + 1], downsampleFramebuffers[i]);
    }
}

void DiffusionCurveRenderer::UpsampleRenderer::BlitSourceFramebuffer(QOpenGLFramebufferObject* source)
{
    for (int attachment = 0; attachment < 2; attachment++)
    {
        QOpenGLFramebufferObject::blitFramebuffer(
            mUpsampleFramebuffers.last(),
            QRect(0, 0, mUpsampleFramebuffers.last()->width(), mUpsampleFramebuffers.last()->height()),
            source,
            QRect(0, 0, source->width(), source->height()),
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR,
            attachment,
            attachment);
    }
}

void DiffusionCurveRenderer::UpsampleRenderer::Upsample(QOpenGLFramebufferObject* target, QOpenGLFramebufferObject* temporary, QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* constraint)
{
    target->bind();
    glViewport(0, 0, target->width(), target->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mUpsampleShader->Bind();
    mUpsampleShader->SetSampler("colorSourceTexture", 0, source->textures().at(0));
    mUpsampleShader->SetSampler("colorTargetTexture", 1, constraint->textures().at(0));
    mUpsampleShader->SetSampler("blurSourceTexture", 2, source->textures().at(1));
    mUpsampleShader->SetSampler("blurTargetTexture", 3, constraint->textures().at(1));
    mQuad->Render();
    mUpsampleShader->Release();

    for (int j = 0; j < mSmoothIterations; j++)
    {
        if (j % 2 == 0)
        {
            temporary->bind();
            glViewport(0, 0, temporary->width(), temporary->height());

            mJacobiShader->Bind();
            mJacobiShader->SetSampler("colorConstrainedTexture", 0, constraint->textures().at(0));
            mJacobiShader->SetSampler("colorTargetTexture", 1, target->textures().at(0));
            mJacobiShader->SetSampler("blurConstrainedTexture", 2, constraint->textures().at(1));
            mJacobiShader->SetSampler("blurTargetTexture", 3, target->textures().at(1));
            mQuad->Render();
            mJacobiShader->Release();
            temporary->release();
        }

        else
        {
            target->bind();
            glViewport(0, 0, target->width(), target->height());

            mJacobiShader->Bind();
            mJacobiShader->SetSampler("colorConstrainedTexture", 0, constraint->textures().at(0));
            mJacobiShader->SetSampler("colorTargetTexture", 1, temporary->textures().at(0));
            mJacobiShader->SetSampler("blurConstrainedTexture", 2, constraint->textures().at(1));
            mJacobiShader->SetSampler("blurTargetTexture", 3, temporary->textures().at(1));
            mQuad->Render();
            mJacobiShader->Release();
            target->release();
        }
    }
}

void DiffusionCurveRenderer::UpsampleRenderer::DeleteFramebuffers()
{
    for (int i = 0; i < mUpsampleFramebuffers.size(); ++i)
    {
        delete mUpsampleFramebuffers[i];
    }

    for (int i = 0; i < mTemporaryFramebuffers.size(); ++i)
    {
        delete mTemporaryFramebuffers[i];
    }

    mUpsampleFramebuffers.clear();
    mTemporaryFramebuffers.clear();
}

void DiffusionCurveRenderer::UpsampleRenderer::CreateFramebuffers()
{
    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    int size = mFramebufferSize;

    while (size > 2)
    {
        mUpsampleFramebuffers << new QOpenGLFramebufferObject(size, size, mFramebufferFormat);
        mUpsampleFramebuffers.last()->addColorAttachment(size, size); // For blur
        mUpsampleFramebuffers.last()->bind();
        glDrawBuffers(2, ATTACHMENTS);
        mUpsampleFramebuffers.last()->release();

        mTemporaryFramebuffers << new QOpenGLFramebufferObject(size, size, mFramebufferFormat);
        mTemporaryFramebuffers.last()->addColorAttachment(size, size); // For blur
        mTemporaryFramebuffers.last()->bind();
        glDrawBuffers(2, ATTACHMENTS);
        mTemporaryFramebuffers.last()->release();

        size /= 2;
    }
}
