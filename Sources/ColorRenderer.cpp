#include "ColorRenderer.h"

DiffusionCurveRenderer::ColorRenderer::ColorRenderer() {}

void DiffusionCurveRenderer::ColorRenderer::Render(QOpenGLFramebufferObject* draw)
{
    auto curves = mCurveManager->GetCurves();

    mCamera->Resize(draw->width(), draw->height());
    mCamera->SetLeft(mCamera->GetLeft() * mQualityFactor);
    mCamera->SetTop(mCamera->GetTop() * mQualityFactor);

    draw->bind();
    glViewport(0, 0, draw->width(), draw->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->Bind(ShaderType::ColorShader);
    mPoints->Bind();

    mShaderManager->SetUniformValue("projection", mCamera->GetProjection());
    mShaderManager->SetUniformValue("pointsDelta", mPoints->GetDelta());
    mShaderManager->SetUniformValue("zoom", mCamera->GetZoom());

    for (auto& curve : curves)
    {
        if (curve == nullptr)
            continue;

        if (curve->mVoid)
            continue;

        curve->Scale(mQualityFactor);

        auto controlPoints = curve->GetControlPointPositions();
        auto leftColors = curve->GetLeftColors();
        auto leftColorPositions = curve->GetLeftColorPositions();
        auto rightColors = curve->GetRightColors();
        auto rightColorPositions = curve->GetRightColorPositions();
        auto blurPointPositions = curve->GetBlurPointPositions();
        auto blurPointStrengths = curve->GetBlurPointStrengths();

        mShaderManager->SetUniformValue("diffusionWidth", mQualityFactor * curve->mDiffusionWidth);
        mShaderManager->SetUniformValue("diffusionGap", mQualityFactor * curve->mDiffusionGap);
        mShaderManager->SetUniformValueArray("controlPoints", controlPoints);
        mShaderManager->SetUniformValue("controlPointsCount", (int)controlPoints.size());
        mShaderManager->SetUniformValueArray("leftColors", leftColors);
        mShaderManager->SetUniformValueArray("leftColorPositions", leftColorPositions);
        mShaderManager->SetUniformValue("leftColorsCount", (int)leftColorPositions.size());
        mShaderManager->SetUniformValueArray("rightColors", rightColors);
        mShaderManager->SetUniformValueArray("rightColorPositions", rightColorPositions);
        mShaderManager->SetUniformValue("rightColorsCount", (int)rightColorPositions.size());
        mShaderManager->SetUniformValueArray("blurPointPositions", blurPointPositions);
        mShaderManager->SetUniformValueArray("blurPointStrengths", blurPointStrengths);
        mShaderManager->SetUniformValue("blurPointsCount", (int)blurPointPositions.size());

        glDrawArrays(GL_POINTS, 0, mPoints->GetSize());

        curve->Scale(1.0f / mQualityFactor);
    }

    mPoints->Release();
    mShaderManager->Release();

    // Restore camera
    mCamera->Resize(mWidth, mHeight);
    mCamera->SetLeft(mCamera->GetLeft() / mQualityFactor);
    mCamera->SetTop(mCamera->GetTop() / mQualityFactor);

    draw->release();
}