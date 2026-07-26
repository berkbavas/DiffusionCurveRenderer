#include "CurveSelectionRenderer.h"

#include "Core/Constants.h"
#include "Util/Chronometer.h"

DiffusionCurveRenderer::CurveSelectionRenderer::CurveSelectionRenderer()
{
    initializeOpenGLFunctions();

    mCurveSelectionShader = new Shader("Curve Selection Shader");
    mCurveSelectionShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bezier.vert");
    mCurveSelectionShader->AddPath(QOpenGLShader::Geometry, ":/Resources/Shaders/CurveSelection.geom");
    mCurveSelectionShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/CurveSelection.frag");
    mCurveSelectionShader->Initialize();

    mInterval = new Interval(0, 1, NUMBER_OF_INTERVALS);

    mFramebuffer = std::make_shared<CurveSelectionFramebuffer>(INITIAL_WIDTH, INITIAL_HEIGHT);
}

void DiffusionCurveRenderer::CurveSelectionRenderer::Render()
{
    MEASURE_CALL_TIME(CURVE_SELECTION_RENDERER);

    const auto& Curves = mCurveContainer->GetCurves();

    if (Curves.isEmpty())
    {
        return;
    }

    mFramebuffer->Clear();
    mFramebuffer->Bind();

    mCurveSelectionShader->Bind();
    mCurveSelectionShader->SetUniformValue("uProjection", mCamera->GetProjectionMatrix());
    mCurveSelectionShader->SetUniformValue("uZoom", mCamera->GetZoom());
    mCurveSelectionShader->SetUniformValue("uDelta", mInterval->GetDelta());
    mCurveSelectionShader->SetUniformValue("uThickness", mCurveSelectionWidth);

    mInterval->Bind();

    for (int Index = 0; Index < Curves.size(); ++Index)
    {
        const auto& pCurve = Curves[Index];

        mCurveSelectionShader->SetUniformValue("uCurveIndex", Index);

        if (const auto pBezier = std::dynamic_pointer_cast<Bezier>(pCurve))
        {
            mCurveSelectionShader->SetUniformValue("uCurveType", 0);
            mCurveSelectionShader->SetUniformValue("uNumberOfControlPoints", static_cast<int>(pBezier->GetNumberOfControlPoints()));
            mCurveSelectionShader->SetUniformValueArray("uControlPoints", pBezier->GetControlPointPositions());
            mInterval->Render();
        }
        else if (const auto pSpline = std::dynamic_pointer_cast<Spline>(pCurve))
        {
            mCurveSelectionShader->SetUniformValue("uCurveType", 1);

            const auto patches = pSpline->GetBezierPatches();

            for (const auto& pBezier : patches)
            {
                mCurveSelectionShader->SetUniformValue("uNumberOfControlPoints", static_cast<int>(pBezier->GetNumberOfControlPoints()));
                mCurveSelectionShader->SetUniformValueArray("uControlPoints", pBezier->GetControlPointPositions());
                mInterval->Render();
            }
        }
        else
        {
            DCR_EXIT_FAILURE("CurveSelectionRenderer::Render: Undefined curve type. Implement this branch!");
        }
    }

    mInterval->Release();
    mCurveSelectionShader->Release();
}

DiffusionCurveRenderer::CurveQueryInfo DiffusionCurveRenderer::CurveSelectionRenderer::Query(const QPoint& QueryPoint)
{
    // Scale query point by device pixel ratio for high DPI displays
    const auto PixelRatio = mCamera->GetPixelRatio();
    const QPoint ScaledPoint(QueryPoint.x() * PixelRatio, QueryPoint.y() * PixelRatio);
    return mFramebuffer->Query(ScaledPoint);
}

void DiffusionCurveRenderer::CurveSelectionRenderer::Resize(int Width, int Height)
{
    mFramebuffer = std::make_shared<CurveSelectionFramebuffer>(Width, Height);
}
