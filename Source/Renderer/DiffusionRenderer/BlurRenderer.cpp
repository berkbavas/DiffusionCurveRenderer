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
    mFramebufferFormat.setMipmap(false);
    mFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    CreateFramebuffer();
}

void DiffusionCurveRenderer::BlurRenderer::Blur(QOpenGLFramebufferObject* framebuffer)
{
    MEASURE_CALL_TIME(BLUR_RENDERER);

    LastBlurPass(framebuffer);
    Combine(framebuffer);
}

void DiffusionCurveRenderer::BlurRenderer::DeleteFramebuffer()
{
    if (mFramebuffer != nullptr)
    {
        delete mFramebuffer;
        mFramebuffer = nullptr;
    }
}

void DiffusionCurveRenderer::BlurRenderer::CreateFramebuffer()
{
    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    mFramebuffer = new QOpenGLFramebufferObject(mFramebufferSize, mFramebufferSize, mFramebufferFormat);
    mFramebuffer->addColorAttachment(mFramebufferSize, mFramebufferSize, GL_RGBA32F);
    mFramebuffer->bind();
    glDrawBuffers(2, ATTACHMENTS);
    mFramebuffer->release();
}

void DiffusionCurveRenderer::BlurRenderer::LastBlurPass(QOpenGLFramebufferObject* framebuffer)
{
    const auto& curves = mCurveContainer->GetCurves();

    if (curves.isEmpty())
    {
        return;
    }

    framebuffer->bind();
    glViewport(0, 0, framebuffer->width(), framebuffer->height());

    mLastBlurPassShader->Bind();
    mInterval->Bind();

    mLastBlurPassShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mLastBlurPassShader->SetUniformValue("delta", mInterval->GetDelta());
    mLastBlurPassShader->SetUniformValue("zoom", mCamera->GetZoom());

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
    framebuffer->release();
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
