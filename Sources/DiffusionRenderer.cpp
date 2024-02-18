#include "DiffusionRenderer.h"

DiffusionCurveRenderer::DiffusionRenderer::DiffusionRenderer()
    : mSmoothIterations(20)
{}

void DiffusionCurveRenderer::DiffusionRenderer::Init()
{
    RendererBase::Init();

    mDefaultFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mDefaultFramebufferFormat.setSamples(0);
    mDefaultFramebufferFormat.setMipmap(false);
    mDefaultFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mDefaultFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    // For color and blur textures
    mDrawBuffers = new GLenum[2];
    mDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    mDrawBuffers[1] = GL_COLOR_ATTACHMENT1;

    CreateFramebuffers();
}

void DiffusionCurveRenderer::DiffusionRenderer::Render(QOpenGLFramebufferObject* initialFramebuffer, QOpenGLFramebufferObject* target, bool clearTarget)
{
    // Downsample 0
    Downsample(mDownsampleFramebuffers[0], initialFramebuffer);

    // Downsample 1,2,3...
    for (int i = 1; i < mDownsampleFramebuffers.size(); ++i)
    {
        Downsample(mDownsampleFramebuffers[i], mDownsampleFramebuffers[i - 1]);
    }

    // Blit mDownsampleFramebuffers.last() ----> mUpsampleFramebuffers.last()
    QOpenGLFramebufferObject::blitFramebuffer(mUpsampleFramebuffers.last(), //
        QRect(0, 0, mUpsampleFramebuffers.last()->width(), mUpsampleFramebuffers.last()->height()),
        mDownsampleFramebuffers.last(),
        QRect(0, 0, mDownsampleFramebuffers.last()->width(), mDownsampleFramebuffers.last()->height()),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST,
        0,
        0);

    QOpenGLFramebufferObject::blitFramebuffer(mUpsampleFramebuffers.last(), //
        QRect(0, 0, mUpsampleFramebuffers.last()->width(), mUpsampleFramebuffers.last()->height()),
        mDownsampleFramebuffers.last(),
        QRect(0, 0, mDownsampleFramebuffers.last()->width(), mDownsampleFramebuffers.last()->height()),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST,
        1,
        1);

    // Upsample and Smooth
    for (int i = mUpsampleFramebuffers.size() - 2; 0 <= i; --i)
    {
        Upsample(mUpsampleFramebuffers[i], mTemporaryFrameBuffers[i], mUpsampleFramebuffers[i + 1], mDownsampleFramebuffers[i]);
    }

    // Last Pass Blur
    DrawFinalBlurCurves(mUpsampleFramebuffers[0]);

    // Post processing (now we apply the actual blur and create the final image)
    if (target)
    {
        target->bind();
        glViewport(0, 0, target->width(), target->height());

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        mShaderManager->Bind(ShaderType::BlurShader);
        mShaderManager->SetSampler("colorTexture", 0, mUpsampleFramebuffers[0]->textures().at(0));
        mShaderManager->SetSampler("blurTexture", 1, mUpsampleFramebuffers[0]->textures().at(1));
        mShaderManager->SetUniformValue("widthRatio", mQualityFactor * float(target->width()) / mUpsampleFramebuffers[0]->width());
        mShaderManager->SetUniformValue("heightRatio", mQualityFactor * float(target->height()) / mUpsampleFramebuffers[0]->height());
        mQuad->Render();
        mShaderManager->Release();
        target->release();

        // Restore
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);
    }
    else
    {
        // Else render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        mShaderManager->Bind(ShaderType::BlurShader);
        mShaderManager->SetSampler("colorTexture", 0, mUpsampleFramebuffers[0]->textures().at(0));
        mShaderManager->SetSampler("blurTexture", 1, mUpsampleFramebuffers[0]->textures().at(1));
        mShaderManager->SetUniformValue("widthRatio", mQualityFactor * float(mWidth) / mUpsampleFramebuffers[0]->width());
        mShaderManager->SetUniformValue("heightRatio", mQualityFactor * float(mHeight) / mUpsampleFramebuffers[0]->height());
        mQuad->Render();
        mShaderManager->Release();
    }
}

void DiffusionCurveRenderer::DiffusionRenderer::Downsample(QOpenGLFramebufferObject* draw, QOpenGLFramebufferObject* read)
{
    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mShaderManager->Bind(ShaderType::DownsampleShader);
    mShaderManager->SetSampler("colorTexture", 0, read->textures().at(0));
    mShaderManager->SetSampler("blurTexture", 1, read->textures().at(1));
    mQuad->Render();
    mShaderManager->Release();
    draw->release();
}

void DiffusionCurveRenderer::DiffusionRenderer::Upsample(QOpenGLFramebufferObject* draw, QOpenGLFramebufferObject* drawBuffer, QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* target)
{
    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->Bind(ShaderType::UpsampleShader);
    mShaderManager->SetSampler("colorSourceTexture", 0, source->textures().at(0));
    mShaderManager->SetSampler("colorTargetTexture", 1, target->textures().at(0));
    mShaderManager->SetSampler("blurSourceTexture", 2, source->textures().at(1));
    mShaderManager->SetSampler("blurTargetTexture", 3, target->textures().at(1));
    mQuad->Render();
    mShaderManager->Release();

    for (int j = 0; j < mSmoothIterations; j++)
    {
        if (j % 2 == 0)
        {
            drawBuffer->bind();
            glViewport(0, 0, drawBuffer->width(), drawBuffer->height());

            mShaderManager->Bind(ShaderType::JacobiShader);
            mShaderManager->SetSampler("colorConstrainedTexture", 0, target->textures().at(0));
            mShaderManager->SetSampler("colorTargetTexture", 1, draw->textures().at(0));
            mShaderManager->SetSampler("blurConstrainedTexture", 2, target->textures().at(1));
            mShaderManager->SetSampler("blurTargetTexture", 3, draw->textures().at(1));
            mQuad->Render();
            mShaderManager->Release();
            drawBuffer->release();
        }

        else
        {
            draw->bind();
            glViewport(0, 0, draw->width(), draw->height());

            mShaderManager->Bind(ShaderType::JacobiShader);
            mShaderManager->SetSampler("colorConstrainedTexture", 0, target->textures().at(0));
            mShaderManager->SetSampler("colorTargetTexture", 1, drawBuffer->textures().at(0));
            mShaderManager->SetSampler("blurConstrainedTexture", 2, target->textures().at(1));
            mShaderManager->SetSampler("blurTargetTexture", 3, drawBuffer->textures().at(1));
            mQuad->Render();
            mShaderManager->Release();
            draw->release();
        }
    }

    //        QOpenGLFramebufferObject::blitFramebuffer(drawBuffer, //
    //                                                  QRect(0, 0, drawBuffer->width(), drawBuffer->height()),
    //                                                  draw,
    //                                                  QRect(0, 0, draw->width(), draw->height()),
    //                                                  GL_COLOR_BUFFER_BIT,
    //                                                  GL_NEAREST,
    //                                                  0,
    //                                                  0);

    //        QOpenGLFramebufferObject::blitFramebuffer(drawBuffer, //
    //                                                  QRect(0, 0, drawBuffer->width(), drawBuffer->height()),
    //                                                  draw,
    //                                                  QRect(0, 0, draw->width(), draw->height()),
    //                                                  GL_COLOR_BUFFER_BIT,
    //                                                  GL_NEAREST,
    //                                                  1,
    //                                                  1);
}

void DiffusionCurveRenderer::DiffusionRenderer::DrawFinalBlurCurves(QOpenGLFramebufferObject* draw)
{
    auto curves = mCurveManager->GetCurves();
    mCamera->Resize(draw->width(), draw->height());

    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());

    mShaderManager->Bind(ShaderType::LastPassBlurShader);
    mPoints->Bind();

    mShaderManager->SetUniformValue("projection", mCamera->GetProjection());
    mShaderManager->SetUniformValue("pointsDelta", mPoints->GetDelta());
    mShaderManager->SetUniformValue("zoom", mCamera->GetZoom());

    for (auto& curve : curves)
    {
        if (curve == nullptr)
            continue;

        auto controlPoints = curve->GetControlPointPositions();
        auto blurPointPositions = curve->GetBlurPointPositions();
        auto blurPointStrengths = curve->GetBlurPointStrengths();

        mShaderManager->SetUniformValue("diffusionWidth", curve->mDiffusionWidth);
        mShaderManager->SetUniformValueArray("controlPoints", controlPoints);
        mShaderManager->SetUniformValue("controlPointsCount", (int)controlPoints.size());
        mShaderManager->SetUniformValueArray("blurPointPositions", blurPointPositions);
        mShaderManager->SetUniformValueArray("blurPointStrengths", blurPointStrengths);
        mShaderManager->SetUniformValue("blurPointsCount", (int)blurPointPositions.size());

        glDrawArrays(GL_POINTS, 0, mPoints->GetSize());
    }

    mPoints->Release();
    mShaderManager->Release();
    draw->release();

    // Restore camera
    mCamera->Resize(mWidth, mHeight);
}

void DiffusionCurveRenderer::DiffusionRenderer::DeleteFramebuffers()
{
    for (int i = 0; i < mDownsampleFramebuffers.size(); ++i)
        delete mDownsampleFramebuffers[i];

    for (int i = 0; i < mUpsampleFramebuffers.size(); ++i)
        delete mUpsampleFramebuffers[i];

    for (int i = 0; i < mTemporaryFrameBuffers.size(); ++i)
        delete mTemporaryFrameBuffers[i];

    mDownsampleFramebuffers.clear();
    mUpsampleFramebuffers.clear();
    mTemporaryFrameBuffers.clear();
}

void DiffusionCurveRenderer::DiffusionRenderer::CreateFramebuffers()
{
    int size = mQualityFactor * qMax(mWidth, mHeight);

    glDrawBuffers(2, mDrawBuffers);

    while (size != 0)
    {
        mDownsampleFramebuffers << new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
        mDownsampleFramebuffers.last()->addColorAttachment(size, size); // For blur info
        mDownsampleFramebuffers.last()->bind();
        glDrawBuffers(2, mDrawBuffers);
        mDownsampleFramebuffers.last()->release();

        mUpsampleFramebuffers << new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
        mUpsampleFramebuffers.last()->addColorAttachment(size, size); // For blur info
        mUpsampleFramebuffers.last()->bind();
        glDrawBuffers(2, mDrawBuffers);
        mUpsampleFramebuffers.last()->release();

        mTemporaryFrameBuffers << new QOpenGLFramebufferObject(size, size, mDefaultFramebufferFormat);
        mTemporaryFrameBuffers.last()->addColorAttachment(size, size); // For blur info
        mTemporaryFrameBuffers.last()->bind();
        glDrawBuffers(2, mDrawBuffers);
        mTemporaryFrameBuffers.last()->release();

        size /= 2;
    }
}

void DiffusionCurveRenderer::DiffusionRenderer::Resize(int w, int h)
{
    RendererBase::Resize(w, h);

    DeleteFramebuffers();
    CreateFramebuffers();
}

void DiffusionCurveRenderer::DiffusionRenderer::SetQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;

    DeleteFramebuffers();
    CreateFramebuffers();
}

void DiffusionCurveRenderer::DiffusionRenderer::SetPixelRatio(float newPixelRatio)
{
    if (qFuzzyCompare(mPixelRatio, newPixelRatio))
        return;

    mPixelRatio = newPixelRatio;

    DeleteFramebuffers();
    CreateFramebuffers();
}

void DiffusionCurveRenderer::DiffusionRenderer::SetSmoothIterations(int newSmoothIterations)
{
    mSmoothIterations = newSmoothIterations;
}