#include "ContourRenderer.h"

#include <QOpenGLFramebufferObject>

DiffusionCurveRenderer::ContourRenderer::ContourRenderer() {}

void DiffusionCurveRenderer::ContourRenderer::Render(QOpenGLFramebufferObject* target, bool clearTarget)
{
    auto curves = mCurveManager->GetCurves();

    if (target)
    {
        mCamera->Resize(target->width(), target->height());

        target->bind();
        glViewport(0, 0, target->width(), target->height());

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    else
    {
        // Render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    mShaderManager->Bind(ShaderType::ContourShader);
    mPoints->Bind();
    mShaderManager->SetUniformValue("projection", mCamera->GetProjection());
    mShaderManager->SetUniformValue("pointDelta", mPoints->GetDelta());
    mShaderManager->SetUniformValue("zoom", mCamera->GetZoom());

    for (const auto& curve : curves)
    {
        if (curve == nullptr)
            continue;

        if (curve->mVoid)
            continue;

        QVector<QVector2D> controlPoints = curve->GetControlPointPositions();

        mShaderManager->SetUniformValue("color", curve->mContourColor);
        mShaderManager->SetUniformValue("thickness", curve->mContourThickness);
        mShaderManager->SetUniformValue("controlPointsCount", (int)controlPoints.size());
        mShaderManager->SetUniformValueArray("controlPoints", controlPoints);

        glDrawArrays(GL_POINTS, 0, mPoints->GetSize());
    }

    mPoints->Release();
    mShaderManager->Release();

    if (target)
    {
        // Restore
        target->release();
        mCamera->Resize(mWidth, mHeight);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);
    }
}

void DiffusionCurveRenderer::ContourRenderer::Render(QOpenGLFramebufferObject* target, Bezier* curve, bool clearTarget)
{
    if (target)
    {
        mCamera->Resize(target->width(), target->height());

        target->bind();
        glViewport(0, 0, target->width(), target->height());

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    else
    {
        // Render to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

        if (clearTarget)
        {
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    mShaderManager->Bind(ShaderType::ContourShader);
    mPoints->Bind();
    mShaderManager->SetUniformValue("projection", mCamera->GetProjection());
    mShaderManager->SetUniformValue("pointDelta", mPoints->GetDelta());
    mShaderManager->SetUniformValue("zoom", mCamera->GetZoom());

    if (curve)
    {
        QVector<QVector2D> controlPoints = curve->GetControlPointPositions();
        mShaderManager->SetUniformValue("color", curve->mContourColor);
        mShaderManager->SetUniformValue("thickness", curve->mContourThickness);
        mShaderManager->SetUniformValue("controlPointsCount", (int)controlPoints.size());
        mShaderManager->SetUniformValueArray("controlPoints", controlPoints);

        glDrawArrays(GL_POINTS, 0, mPoints->GetSize());
    }

    mPoints->Release();
    mShaderManager->Release();

    if (target)
    {
        // Restore
        target->release();
        mCamera->Resize(mWidth, mHeight);
        glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);
    }
}