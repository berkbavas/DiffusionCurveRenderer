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
}

void DiffusionCurveRenderer::ColorRenderer::Render(QOpenGLFramebufferObject* framebuffer)
{
    MEASURE_CALL_TIME(COLOR_RENDERER);

    const auto& curves = mCurveContainer->GetCurves();

    if (curves.isEmpty())
    {
        return;
    }

    framebuffer->bind();
    glViewport(0, 0, framebuffer->width(), framebuffer->height());

    mColorShader->Bind();
    mColorShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mColorShader->SetUniformValue("delta", mInterval->GetDelta());
    mColorShader->SetUniformValue("zoom", mCamera->GetZoom());

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
            DCR_EXIT_FAILURE("ColoringRenderer::Render: Undefined curve type. Implement this branch!");
        }
    }

    mInterval->Release();
    mColorShader->Release();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
