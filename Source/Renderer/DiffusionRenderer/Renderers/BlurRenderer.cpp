#include "BlurRenderer.h"

#include "Util/Chronometer.h"

DiffusionCurveRenderer::BlurRenderer::BlurRenderer()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;
    mInterval = new Interval(0, 1, NUMBER_OF_INTERVALS);

    mBlurShader = new Shader("Blur Shader");
    mBlurShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mBlurShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Blur.frag");
    mBlurShader->Initialize();

    mLastBlurPassShader = new Shader("Last Blur Pass");
    mLastBlurPassShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bezier.vert");
    mLastBlurPassShader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/LastBlurPass.geom");
    mLastBlurPassShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/LastBlurPass.frag");
    mLastBlurPassShader->Initialize();

    mFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFramebufferFormat.setSamples(0);

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::BlurRenderer::Blur(QOpenGLFramebufferObject* target, QOpenGLFramebufferObject* source)
{
    MEASURE_CALL_TIME(BLUR_RENDERER);

    LastBlurPass(source);
    Combine(source);

    QOpenGLFramebufferObject::blitFramebuffer(target,
                                              QRect(0, 0, target->width(), target->height()),
                                              mFramebuffer.get(),
                                              QRect(0, 0, mFramebuffer->width(), mFramebuffer->height()),
                                              GL_COLOR_BUFFER_BIT,
                                              GL_LINEAR,
                                              0,
                                              0);
}

void DiffusionCurveRenderer::BlurRenderer::SetFramebufferSize(int size)
{
    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    mFramebuffer = std::make_unique<QOpenGLFramebufferObject>(size, size, mFramebufferFormat);
    mFramebuffer->addColorAttachment(size, size);
    mFramebuffer->bind();
    glDrawBuffers(2, ATTACHMENTS);
    mFramebuffer->release();
}

void DiffusionCurveRenderer::BlurRenderer::LastBlurPass(QOpenGLFramebufferObject* source)
{
    source->bind();
    glViewport(0, 0, source->width(), source->height());

    mLastBlurPassShader->Bind();
    mInterval->Bind();

    mLastBlurPassShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mLastBlurPassShader->SetUniformValue("delta", mInterval->GetDelta());

    const auto& curves = mCurveContainer->GetCurves();

    for (const auto& curve : curves)
    {

        if (const auto spline = std::dynamic_pointer_cast<Spline>(curve))
        {

            mLastBlurPassShader->SetUniformValue("diffusionWidth", spline->GetDiffusionWidth());

            const auto patches = spline->GetBezierPatches();

            for (const auto& bezier : patches)
            {

                mLastBlurPassShader->SetUniformValueArray("controlPoints", bezier->GetControlPointPositions());
                mLastBlurPassShader->SetUniformValue("controlPointsCount", bezier->GetNumberOfControlPoints());
                mLastBlurPassShader->SetUniformValueFloatArray("blurPointPositions", bezier->GetBlurPointPositions());
                mLastBlurPassShader->SetUniformValueFloatArray("blurPointStrengths", bezier->GetBlurPointStrengths());
                mLastBlurPassShader->SetUniformValue("blurPointsCount", bezier->GetNumberOfBlurPoints());
                mInterval->Render();
            }
        }
        else if (const auto bezier = std::dynamic_pointer_cast<Bezier>(curve))
        {
            mLastBlurPassShader->SetUniformValue("diffusionWidth", bezier->GetDiffusionWidth());
            mLastBlurPassShader->SetUniformValueArray("controlPoints", bezier->GetControlPointPositions());
            mLastBlurPassShader->SetUniformValue("controlPointsCount", bezier->GetNumberOfControlPoints());
            mLastBlurPassShader->SetUniformValueFloatArray("blurPointPositions", bezier->GetBlurPointPositions());
            mLastBlurPassShader->SetUniformValueFloatArray("blurPointStrengths", bezier->GetBlurPointStrengths());
            mLastBlurPassShader->SetUniformValue("blurPointsCount", bezier->GetNumberOfBlurPoints());
            mInterval->Render();
        }
        else
        {
            DCR_EXIT_FAILURE("BlurRenderer::LastBlurPass: Undefined curve type. Implement this branch!");
        }
    }

    mInterval->Release();
    mLastBlurPassShader->Release();
    source->release();
}

void DiffusionCurveRenderer::BlurRenderer::Combine(QOpenGLFramebufferObject* source)
{
    mFramebuffer->bind();
    glViewport(0, 0, mFramebuffer->width(), mFramebuffer->height());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    mBlurShader->Bind();
    mBlurShader->SetSampler("colorTexture", 0, source->textures().at(0));
    mBlurShader->SetSampler("blurTexture", 1, source->textures().at(1));
    mQuad->Render();
    mBlurShader->Release();
    mFramebuffer->release();
}
