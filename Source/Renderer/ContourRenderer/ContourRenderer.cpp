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

void DiffusionCurveRenderer::ContourRenderer::Render(QOpenGLFramebufferObject* pTarget)
{
    MEASURE_CALL_TIME(CONTOUR_RENDERER);

    if (pTarget == nullptr)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pTarget->handle());
        glViewport(0, 0, pTarget->width(), pTarget->height());
    }

    mBezierShader->Bind();
    mBezierShader->SetUniformValue("uProjection", mCamera->GetProjectionMatrix());
    mBezierShader->SetUniformValue("uDelta", mInterval->GetDelta());

    mInterval->Bind();

    const auto& Curves = mCurveContainer->GetCurves();

    for (const auto& pCurve : Curves)
    {
        RenderCurveInner(pCurve);
    }

    mInterval->Release();
    mBezierShader->Release();
}

void DiffusionCurveRenderer::ContourRenderer::RenderCurve(CurvePtr pCurve, QOpenGLFramebufferObject* pTarget)
{
    if (pTarget == nullptr)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pTarget->handle());
        glViewport(0, 0, pTarget->width(), pTarget->height());
    }

    mBezierShader->Bind();
    mBezierShader->SetUniformValue("uProjection", mCamera->GetProjectionMatrix());
    mBezierShader->SetUniformValue("uDelta", mInterval->GetDelta());

    mInterval->Bind();
    RenderCurveInner(pCurve);
    mInterval->Release();

    mBezierShader->Release();
}

void DiffusionCurveRenderer::ContourRenderer::RenderCurveInner(CurvePtr pCurve)
{
    if (const auto pBezier = std::dynamic_pointer_cast<Bezier>(pCurve))
    {
        mBezierShader->SetUniformValue("uThickness", pBezier->GetContourThickness());
        mBezierShader->SetUniformValue("uColor", pBezier->GetContourColor());
        mBezierShader->SetUniformValue("uNumberOfControlPoints", static_cast<int>(pBezier->GetNumberOfControlPoints()));
        mBezierShader->SetUniformValueArray("uControlPoints", pBezier->GetControlPointPositions());
        mInterval->Render();
    }
    else if (const auto pSpline = std::dynamic_pointer_cast<Spline>(pCurve))
    {
        mBezierShader->SetUniformValue("uThickness", pSpline->GetContourThickness());
        mBezierShader->SetUniformValue("uColor", pSpline->GetContourColor());

        const auto& Patches = pSpline->GetBezierPatches();

        for (const auto& pBezier : Patches)
        {
            mBezierShader->SetUniformValue("uNumberOfControlPoints", static_cast<int>(pBezier->GetNumberOfControlPoints()));
            mBezierShader->SetUniformValueArray("uControlPoints", pBezier->GetControlPointPositions());
            mInterval->Render();
        }
    }
    else
    {
        DCR_EXIT_FAILURE("ContourRenderer::Render: Undefined curve type. Implement this branch!");
    }
}
