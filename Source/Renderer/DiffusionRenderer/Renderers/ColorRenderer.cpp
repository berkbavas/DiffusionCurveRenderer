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

void DiffusionCurveRenderer::ColorRenderer::Render(QOpenGLFramebufferObject* pTarget)
{
    MEASURE_CALL_TIME(COLOR_RENDERER);

    if (mUseMultisampleFramebuffer)
    {
        RenderPrivate(mMultisampleFramebuffer.get());
        BlitFramebuffer(mMultisampleFramebuffer.get(), pTarget);
    }
    else
    {
        RenderPrivate(pTarget);
    }
}

void DiffusionCurveRenderer::ColorRenderer::RenderPrivate(QOpenGLFramebufferObject* pTarget)
{
    pTarget->bind();
    glViewport(0, 0, pTarget->width(), pTarget->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mColorShader->Bind();
    mColorShader->SetUniformValue("uProjection", mCamera->GetProjectionMatrix());
    mColorShader->SetUniformValue("uDelta", mInterval->GetDelta());

    mInterval->Bind();

    const auto& Curves = mCurveContainer->GetCurves();

    for (const auto& pCurve : Curves)
    {
        if (const auto pBezier = std::dynamic_pointer_cast<Bezier>(pCurve))
        {
            mColorShader->SetUniformValue("uDiffusionWidth", pBezier->GetDiffusionWidth());
            mColorShader->SetUniformValue("uDiffusionGap", pBezier->GetDiffusionGap());

            SetUniforms(pBezier);

            mInterval->Render();
        }
        else if (const auto pSpline = std::dynamic_pointer_cast<Spline>(pCurve))
        {
            mColorShader->SetUniformValue("uDiffusionWidth", pSpline->GetDiffusionWidth());
            mColorShader->SetUniformValue("uDiffusionGap", pSpline->GetDiffusionGap());

            const auto& Patches = pSpline->GetBezierPatches();

            for (const auto& pBezier : Patches)
            {
                SetUniforms(pBezier);
                mInterval->Render();
            }
        }
        else
        {
            DCR_EXIT_FAILURE("ColorRenderer::RenderPrivate: Undefined curve type. Implement this branch!");
        }
    }

    mInterval->Release();
    mColorShader->Release();
    pTarget->release();
}

void DiffusionCurveRenderer::ColorRenderer::SetUniforms(BezierPtr pCurve)
{
    mColorShader->SetUniformValueArray("uControlPoints", pCurve->GetControlPointPositions());
    mColorShader->SetUniformValue("uControlPointsCount", pCurve->GetNumberOfControlPoints());
    mColorShader->SetUniformValueArray("uLeftColors", pCurve->GetLeftColors());
    mColorShader->SetUniformValueFloatArray("uLeftColorPositions", pCurve->GetLeftColorPositions());
    mColorShader->SetUniformValue("uLeftColorsCount", pCurve->GetNumberOfLeftColors());
    mColorShader->SetUniformValueArray("uRightColors", pCurve->GetRightColors());
    mColorShader->SetUniformValueFloatArray("uRightColorPositions", pCurve->GetRightColorPositions());
    mColorShader->SetUniformValue("uRightColorsCount", pCurve->GetNumberOfRightColors());
}

void DiffusionCurveRenderer::ColorRenderer::SetFramebufferSize(int Size)
{
    mMultisampleFramebuffer = std::make_unique<QOpenGLFramebufferObject>(Size, Size, mMultisampleFramebufferFormat);
}

void DiffusionCurveRenderer::ColorRenderer::BlitFramebuffer(QOpenGLFramebufferObject* pSource, QOpenGLFramebufferObject* pTarget)
{
    QOpenGLFramebufferObject::blitFramebuffer(pTarget, //
                                              QRect(0, 0, pTarget->width(), pTarget->height()),
                                              pSource,
                                              QRect(0, 0, pSource->width(), pSource->height()),
                                              GL_COLOR_BUFFER_BIT,
                                              GL_LINEAR,
                                              0,
                                              0);
}
