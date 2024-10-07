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

    const auto& curves = mCurveContainer->GetCurves();

    if (curves.isEmpty())
    {
        return;
    }

    mFramebuffer->Clear();
    mFramebuffer->Bind();

    mCurveSelectionShader->Bind();
    mCurveSelectionShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mCurveSelectionShader->SetUniformValue("zoom", mCamera->GetZoom());
    mCurveSelectionShader->SetUniformValue("delta", mInterval->GetDelta());
    mCurveSelectionShader->SetUniformValue("thickness", mCurveSelectionWidth);

    mInterval->Bind();

    for (int index = 0; index < curves.size(); ++index)
    {
        const auto curve = curves[index];

        mCurveSelectionShader->SetUniformValue("curveIndex", index);

        if (const auto bezier = std::dynamic_pointer_cast<Bezier>(curve))
        {
            mCurveSelectionShader->SetUniformValue("curveType", 0);
            mCurveSelectionShader->SetUniformValue("numberOfControlPoints", static_cast<int>(bezier->GetNumberOfControlPoints()));
            mCurveSelectionShader->SetUniformValueArray("controlPoints", bezier->GetControlPointPositions());
            mInterval->Render();
        }
        else if (const auto spline = std::dynamic_pointer_cast<Spline>(curve))
        {
            mCurveSelectionShader->SetUniformValue("curveType", 1);

            const auto patches = spline->GetBezierPatches();

            for (const auto& bezier : patches)
            {
                mCurveSelectionShader->SetUniformValue("numberOfControlPoints", static_cast<int>(bezier->GetNumberOfControlPoints()));
                mCurveSelectionShader->SetUniformValueArray("controlPoints", bezier->GetControlPointPositions());
                mInterval->Render();
            }
        }
        else
        {
            DCR_EXIT_FAILURE("ContourRenderer::Render: Undefined curve type. Implement this branch!");
        }
    }

    mInterval->Release();
    mCurveSelectionShader->Release();
}

DiffusionCurveRenderer::CurveQueryInfo DiffusionCurveRenderer::CurveSelectionRenderer::Query(const QPoint& queryPoint)
{
    return mFramebuffer->Query(queryPoint);
}

void DiffusionCurveRenderer::CurveSelectionRenderer::Resize(int width, int height)
{
    mFramebuffer = std::make_shared<CurveSelectionFramebuffer>(width, height);
}
