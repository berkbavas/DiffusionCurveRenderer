#include "ContourRenderer.h"

#include "Core/Constants.h"
#include "Util/Chronometer.h"

DiffusionCurveRenderer::ContourRenderer::ContourRenderer()
{
    initializeOpenGLFunctions();

    mBezierShader = new Shader("Bezier Shader");
    mBezierShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bezier.vert");
    mBezierShader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/Bezier.geom");
    mBezierShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Bezier.frag");
    mBezierShader->Initialize();

    mInterval = new Interval(0, 1, NUMBER_OF_INTERVALS);
}

void DiffusionCurveRenderer::ContourRenderer::Render(QOpenGLFramebufferObject* framebuffer, QVector4D* globalColorOption)
{
    MEASURE_CALL_TIME(CONTOUR_RENDERER);

    const auto& curves = mCurveContainer->GetCurves();

    if (curves.isEmpty())
    {
        return;
    }

    if (framebuffer == nullptr) // Default framebuffer
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle());
        glViewport(0, 0, framebuffer->width(), framebuffer->height());
    }

    mBezierShader->Bind();
    mBezierShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mBezierShader->SetUniformValue("zoom", mCamera->GetZoom());
    mBezierShader->SetUniformValue("delta", mInterval->GetDelta());

    mInterval->Bind();

    for (const auto& curve : curves)
    {
        RenderCurve(curve);
    }

    mInterval->Release();

    mBezierShader->Release();
}

void DiffusionCurveRenderer::ContourRenderer::RenderCurve(QOpenGLFramebufferObject* framebuffer, CurvePtr curve)
{
    if (framebuffer == nullptr) // Default framebuffer
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle());
        glViewport(0, 0, framebuffer->width(), framebuffer->height());
    }

    mBezierShader->Bind();
    mBezierShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mBezierShader->SetUniformValue("zoom", mCamera->GetZoom());
    mBezierShader->SetUniformValue("delta", mInterval->GetDelta());

    mInterval->Bind();
    RenderCurve(curve);
    mInterval->Release();

    mBezierShader->Release();
}

void DiffusionCurveRenderer::ContourRenderer::RenderCurve(CurvePtr curve)
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