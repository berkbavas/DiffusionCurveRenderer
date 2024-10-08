#include "ColorRenderer.h"

#include "Util/Chronometer.h"

DiffusionCurveRenderer::ColorRenderer::ColorRenderer()
{
    initializeOpenGLFunctions();

    mInterval = new Interval(0, 1, NUMBER_OF_INTERVALS);

    mColorShader = new Shader("Color Shader");
    mColorShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bezier.vert");
    mColorShader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Color.geom");
    mColorShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Color.frag");
    mColorShader->Initialize();

    mMultisampleFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mMultisampleFramebufferFormat.setSamples(8);

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::ColorRenderer::Render(QOpenGLFramebufferObject* target)
{
    MEASURE_CALL_TIME(COLOR_RENDERER);

    if (mUseMultisampleFramebuffer)
    {
        RenderPrivate(mMultisampleFramebuffer.get());
        BlitFramebuffer(mMultisampleFramebuffer.get(), target);
    }
    else
    {
        RenderPrivate(target);
    }
}

void DiffusionCurveRenderer::ColorRenderer::RenderPrivate(QOpenGLFramebufferObject* target)
{
    const auto& curves = mCurveContainer->GetCurves();

    if (curves.isEmpty())
    {
        return;
    }

    target->bind();
    glViewport(0, 0, target->width(), target->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mColorShader->Bind();
    mColorShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mColorShader->SetUniformValue("delta", mInterval->GetDelta());

    mInterval->Bind();

    for (const auto& curve : curves)
    {
        if (const auto bezier = std::dynamic_pointer_cast<Bezier>(curve))
        {
            mColorShader->SetUniformValue("diffusionWidth", curve->GetDiffusionWidth());
            mColorShader->SetUniformValue("diffusionGap", curve->GetDiffusionGap());

            SetUniforms(bezier);

            mInterval->Render();
        }
        else if (const auto spline = std::dynamic_pointer_cast<Spline>(curve))
        {
            mColorShader->SetUniformValue("diffusionWidth", spline->GetDiffusionWidth());
            mColorShader->SetUniformValue("diffusionGap", spline->GetDiffusionGap());

            const auto patches = spline->GetBezierPatches();

            for (const auto& bezier : patches)
            {
                SetUniforms(bezier);

                mInterval->Render();
            }
        }
        else
        {
            DCR_EXIT_FAILURE("ColoringRenderer::RenderPrivate: Undefined curve type. Implement this branch!");
        }
    }

    mInterval->Release();
    mColorShader->Release();
    target->release();
}

void DiffusionCurveRenderer::ColorRenderer::SetUniforms(BezierPtr curve)
{
    mColorShader->SetUniformValueArray("controlPoints", curve->GetControlPointPositions());
    mColorShader->SetUniformValue("controlPointsCount", curve->GetNumberOfControlPoints());
    mColorShader->SetUniformValueArray("leftColors", curve->GetLeftColors());
    mColorShader->SetUniformValueFloatArray("leftColorPositions", curve->GetLeftColorPositions());
    mColorShader->SetUniformValue("leftColorsCount", curve->GetNumberOfLeftColors());
    mColorShader->SetUniformValueArray("rightColors", curve->GetRightColors());
    mColorShader->SetUniformValueFloatArray("rightColorPositions", curve->GetRightColorPositions());
    mColorShader->SetUniformValue("rightColorsCount", curve->GetNumberOfRightColors());
    mColorShader->SetUniformValueFloatArray("blurPointPositions", curve->GetBlurPointPositions());
    mColorShader->SetUniformValueFloatArray("blurPointStrengths", curve->GetBlurPointStrengths());
    mColorShader->SetUniformValue("blurPointsCount", curve->GetNumberOfBlurPoints());
}

void DiffusionCurveRenderer::ColorRenderer::SetFramebufferSize(int size)
{
    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    mMultisampleFramebuffer = std::make_unique<QOpenGLFramebufferObject>(size, size, mMultisampleFramebufferFormat);
    mMultisampleFramebuffer->addColorAttachment(size, size);
    mMultisampleFramebuffer->bind();
    glDrawBuffers(2, ATTACHMENTS);
    mMultisampleFramebuffer->release();
}

void DiffusionCurveRenderer::ColorRenderer::BlitFramebuffer(QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* target)
{
    for (int attachment = 0; attachment < 2; attachment++)
    {
        QOpenGLFramebufferObject::blitFramebuffer(target,
                                                  QRect(0, 0, target->width(), target->height()),
                                                  source,
                                                  QRect(0, 0, source->width(), source->height()),
                                                  GL_COLOR_BUFFER_BIT,
                                                  GL_LINEAR,
                                                  attachment,
                                                  attachment);
    }
}
