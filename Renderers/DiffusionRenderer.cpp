#include "DiffusionRenderer.h"

DiffusionRenderer::DiffusionRenderer()
    : mSmoothIterations(20)
{}

void DiffusionRenderer::init()
{
    RendererBase::init();

    mDefaultFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mDefaultFramebufferFormat.setSamples(0);
    mDefaultFramebufferFormat.setMipmap(false);
    mDefaultFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mDefaultFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    // For color and blur textures
    mDrawBuffers = new GLenum[2];
    mDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    mDrawBuffers[1] = GL_COLOR_ATTACHMENT1;

    createFramebuffers();
}

void DiffusionRenderer::render(QOpenGLFramebufferObject *initialFramebuffer, QOpenGLFramebufferObject *target, bool clearTarget)
{
    // Downsample 0
    downsample(mDownsampleFramebuffers[0], initialFramebuffer);

    // Downsample 1,2,3...
    for (int i = 1; i < mDownsampleFramebuffers.size(); ++i)
    {
        downsample(mDownsampleFramebuffers[i], mDownsampleFramebuffers[i - 1]);
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
        upsample(mUpsampleFramebuffers[i], mTemporaryFrameBuffers[i], mUpsampleFramebuffers[i + 1], mDownsampleFramebuffers[i]);
    }

    // Last Pass Blur
    drawFinalBlurCurves(mUpsampleFramebuffers[0]);

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

        mShaderManager->bind(ShaderType::BlurShader);
        mShaderManager->setSampler("colorTexture", 0, mUpsampleFramebuffers[0]->textures().at(0));
        mShaderManager->setSampler("blurTexture", 1, mUpsampleFramebuffers[0]->textures().at(1));
        mShaderManager->setUniformValue("widthRatio", mQualityFactor * float(target->width()) / mUpsampleFramebuffers[0]->width());
        mShaderManager->setUniformValue("heightRatio", mQualityFactor * float(target->height()) / mUpsampleFramebuffers[0]->height());
        mQuad->render();
        mShaderManager->release();
        target->release();

        // Restore
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

    } else
    {
        // Else render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        mShaderManager->bind(ShaderType::BlurShader);
        mShaderManager->setSampler("colorTexture", 0, mUpsampleFramebuffers[0]->textures().at(0));
        mShaderManager->setSampler("blurTexture", 1, mUpsampleFramebuffers[0]->textures().at(1));
        mShaderManager->setUniformValue("widthRatio", mQualityFactor * float(mWidth) / mUpsampleFramebuffers[0]->width());
        mShaderManager->setUniformValue("heightRatio", mQualityFactor * float(mHeight) / mUpsampleFramebuffers[0]->height());
        mQuad->render();
        mShaderManager->release();
    }
}

void DiffusionRenderer::downsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *read)
{
    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    mShaderManager->bind(ShaderType::DownsampleShader);
    mShaderManager->setSampler("colorTexture", 0, read->textures().at(0));
    mShaderManager->setSampler("blurTexture", 1, read->textures().at(1));
    mQuad->render();
    mShaderManager->release();
    draw->release();
}

void DiffusionRenderer::upsample(QOpenGLFramebufferObject *draw, QOpenGLFramebufferObject *drawBuffer, QOpenGLFramebufferObject *source, QOpenGLFramebufferObject *target)
{
    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->bind(ShaderType::UpsampleShader);
    mShaderManager->setSampler("colorSourceTexture", 0, source->textures().at(0));
    mShaderManager->setSampler("colorTargetTexture", 1, target->textures().at(0));
    mShaderManager->setSampler("blurSourceTexture", 2, source->textures().at(1));
    mShaderManager->setSampler("blurTargetTexture", 3, target->textures().at(1));
    mQuad->render();
    mShaderManager->release();

    for (int j = 0; j < mSmoothIterations; j++)
    {
        if (j % 2 == 0)
        {
            drawBuffer->bind();
            glViewport(0, 0, drawBuffer->width(), drawBuffer->height());

            mShaderManager->bind(ShaderType::JacobiShader);
            mShaderManager->setSampler("colorConstrainedTexture", 0, target->textures().at(0));
            mShaderManager->setSampler("colorTargetTexture", 1, draw->textures().at(0));
            mShaderManager->setSampler("blurConstrainedTexture", 2, target->textures().at(1));
            mShaderManager->setSampler("blurTargetTexture", 3, draw->textures().at(1));
            mQuad->render();
            mShaderManager->release();
            drawBuffer->release();
        }

        else
        {
            draw->bind();
            glViewport(0, 0, draw->width(), draw->height());

            mShaderManager->bind(ShaderType::JacobiShader);
            mShaderManager->setSampler("colorConstrainedTexture", 0, target->textures().at(0));
            mShaderManager->setSampler("colorTargetTexture", 1, drawBuffer->textures().at(0));
            mShaderManager->setSampler("blurConstrainedTexture", 2, target->textures().at(1));
            mShaderManager->setSampler("blurTargetTexture", 3, drawBuffer->textures().at(1));
            mQuad->render();
            mShaderManager->release();
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

void DiffusionRenderer::drawFinalBlurCurves(QOpenGLFramebufferObject *draw)
{
    auto curves = mCurveManager->curves();
    mCamera->resize(draw->width(), draw->height());

    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());

    mShaderManager->bind(ShaderType::LastPassBlurShader);
    mPoints->bind();

    mShaderManager->setUniformValue("projection", mCamera->projection());
    mShaderManager->setUniformValue("pointsDelta", mPoints->delta());
    mShaderManager->setUniformValue("zoom", mCamera->zoom());

    for (auto &curve : curves)
    {
        if (curve == nullptr)
            continue;

        auto controlPoints = curve->getControlPointPositions();
        auto blurPointPositions = curve->getBlurPointPositions();
        auto blurPointStrengths = curve->getBlurPointStrengths();

        mShaderManager->setUniformValue("diffusionWidth", curve->mDiffusionWidth);
        mShaderManager->setUniformValueArray("controlPoints", controlPoints);
        mShaderManager->setUniformValue("controlPointsCount", (int) controlPoints.size());
        mShaderManager->setUniformValueArray("blurPointPositions", blurPointPositions);
        mShaderManager->setUniformValueArray("blurPointStrengths", blurPointStrengths);
        mShaderManager->setUniformValue("blurPointsCount", (int) blurPointPositions.size());

        glDrawArrays(GL_POINTS, 0, mPoints->size());
    }

    mPoints->release();
    mShaderManager->release();
    draw->release();

    // Restore camera
    mCamera->resize(mWidth, mHeight);
}

void DiffusionRenderer::deleteFramebuffers()
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

void DiffusionRenderer::createFramebuffers()
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

void DiffusionRenderer::resize(int w, int h)
{
    RendererBase::resize(w, h);

    deleteFramebuffers();
    createFramebuffers();
}

void DiffusionRenderer::setQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;

    deleteFramebuffers();
    createFramebuffers();
}

void DiffusionRenderer::setPixelRatio(float newPixelRatio)
{
    if (qFuzzyCompare(mPixelRatio, newPixelRatio))
        return;

    mPixelRatio = newPixelRatio;

    deleteFramebuffers();
    createFramebuffers();
}

void DiffusionRenderer::setSmoothIterations(int newSmoothIterations)
{
    mSmoothIterations = newSmoothIterations;
}
