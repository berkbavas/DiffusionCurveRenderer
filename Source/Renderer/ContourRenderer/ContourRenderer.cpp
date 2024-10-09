#include "ContourRenderer.h"

#include "Core/Constants.h"
#include "Util/Chronometer.h"

void DiffusionCurveRenderer::ContourRenderer::Initialize()
{
    initializeOpenGLFunctions();

    mBezierShader = new Shader("Bezier Shader");
    mBezierShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bezier.vert");
    mBezierShader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Bezier.geom");
    mBezierShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Bezier.frag");
    mBezierShader->Initialize();

    mInterval = new Interval(0, 1, NUMBER_OF_INTERVALS);
}

void DiffusionCurveRenderer::ContourRenderer::Render(QOpenGLFramebufferObject* target)
{
    MEASURE_CALL_TIME(CONTOUR_RENDERER);

    if (target == nullptr)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, target->handle());
        glViewport(0, 0, target->width(), target->height());
    }

    mBezierShader->Bind();
    mBezierShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mBezierShader->SetUniformValue("delta", mInterval->GetDelta());

    mInterval->Bind();

    const auto& curves = mCurveContainer->GetCurves();

    for (const auto& curve : curves)
    {
        RenderCurveInner(curve);
    }

    mInterval->Release();
    mBezierShader->Release();
}

void DiffusionCurveRenderer::ContourRenderer::RenderCurve(CurvePtr curve, QOpenGLFramebufferObject* target)
{
    if (target == nullptr)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, target->handle());
        glViewport(0, 0, target->width(), target->height());
    }

    mBezierShader->Bind();
    mBezierShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mBezierShader->SetUniformValue("delta", mInterval->GetDelta());

    mInterval->Bind();
    RenderCurveInner(curve);
    mInterval->Release();

    mBezierShader->Release();
}

void DiffusionCurveRenderer::ContourRenderer::RenderCurveInner(CurvePtr curve)
{
    if (const auto bezier = std::dynamic_pointer_cast<Bezier>(curve))
    {
        mBezierShader->SetUniformValue("thickness", bezier->GetContourThickness());
        mBezierShader->SetUniformValue("color", bezier->GetContourColor());
        mBezierShader->SetUniformValue("numberOfControlPoints", static_cast<int>(bezier->GetNumberOfControlPoints()));
        mBezierShader->SetUniformValueArray("controlPoints", bezier->GetControlPointPositions());
        mInterval->Render();
    }
    else if (const auto spline = std::dynamic_pointer_cast<Spline>(curve))
    {
        mBezierShader->SetUniformValue("thickness", spline->GetContourThickness());
        mBezierShader->SetUniformValue("color", spline->GetContourColor());

        const auto patches = spline->GetBezierPatches();

        for (const auto& bezier : patches)
        {
            mBezierShader->SetUniformValue("numberOfControlPoints", static_cast<int>(bezier->GetNumberOfControlPoints()));
            mBezierShader->SetUniformValueArray("controlPoints", bezier->GetControlPointPositions());
            mInterval->Render();
        }
    }
    else
    {
        DCR_EXIT_FAILURE("ContourRenderer::Render: Undefined curve type. Implement this branch!");
    }
}
