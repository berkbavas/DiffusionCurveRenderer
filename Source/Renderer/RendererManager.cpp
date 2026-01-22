#include "RendererManager.h"

#include "Core/Constants.h"
#include "Renderer/BitmapRenderer/BitmapRenderer.h"
#include "Renderer/ContourRenderer/ContourRenderer.h"
#include "Renderer/DiffusionRenderer/DiffusionRenderer.h"
#include "Util/Chronometer.h"

#include <QImage>

void DiffusionCurveRenderer::RendererManager::Initialize()
{
    initializeOpenGLFunctions();

    mContourRenderer = new ContourRenderer;
    mContourRenderer->SetCamera(mCamera);
    mContourRenderer->SetCurveContainer(mCurveContainer);
    mContourRenderer->Initialize();

    mDiffusionRenderer = new DiffusionRenderer;
    mDiffusionRenderer->SetCamera(mCamera);
    mDiffusionRenderer->SetCurveContainer(mCurveContainer);
    mDiffusionRenderer->Initialize();

    mCurveSelectionRenderer = new CurveSelectionRenderer;
    mCurveSelectionRenderer->SetCamera(mCamera);
    mCurveSelectionRenderer->SetCurveContainer(mCurveContainer);

    mBitmapRenderer = new BitmapRenderer;
    mBitmapRenderer->SetCamera(mCamera);

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::RendererManager::Resize(int width, int height)
{
    mCurveSelectionRenderer->Resize(width, height);
}

void DiffusionCurveRenderer::RendererManager::Clear()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    glClearColor(mBackgroundColor.x(), mBackgroundColor.y(), mBackgroundColor.z(), mBackgroundColor.w());
    glClear(GL_COLOR_BUFFER_BIT);
}

void DiffusionCurveRenderer::RendererManager::RenderDiffusion()
{
    mDiffusionRenderer->Render();
}

void DiffusionCurveRenderer::RendererManager::RenderContours()
{
    mContourRenderer->Render();
}

void DiffusionCurveRenderer::RendererManager::RenderForCurveSelection()
{
    mCurveSelectionRenderer->Render();
}

void DiffusionCurveRenderer::RendererManager::RenderCurve(CurvePtr curve)
{
    mContourRenderer->RenderCurve(curve);
}

void DiffusionCurveRenderer::RendererManager::Save(const QString& path, RenderModes renderModes)
{
    mSaveFramebuffer = std::make_unique<QOpenGLFramebufferObject>(mCamera->GetWidth(), mCamera->GetHeight());

    // Clear
    glBindFramebuffer(GL_FRAMEBUFFER, mSaveFramebuffer->handle());
    glViewport(0, 0, mSaveFramebuffer->width(), mSaveFramebuffer->height());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (renderModes.testAnyFlags(RenderMode::Diffusion))
        mDiffusionRenderer->Render(mSaveFramebuffer.get());

    if (renderModes.testAnyFlag(RenderMode::Contour))
        mContourRenderer->Render(mSaveFramebuffer.get());

    mSaveFramebuffer->toImage().save(path);
}

void DiffusionCurveRenderer::RendererManager::SetFramebufferSize(int size)
{
    mFramebufferSize = size;
    mDiffusionRenderer->SetFramebufferSize(size);
}

void DiffusionCurveRenderer::RendererManager::SetSmoothIterations(int smoothIterations)
{
    mDiffusionRenderer->SetSmoothIterations(smoothIterations);
}

void DiffusionCurveRenderer::RendererManager::SetUseMultisampleFramebuffer(bool val)
{
    mDiffusionRenderer->SetUseMultisampleFramebuffer(val);
}

int DiffusionCurveRenderer::RendererManager::GetSmoothIterations() const
{
    return mDiffusionRenderer->GetSmoothIterations();
}

DiffusionCurveRenderer::CurveQueryInfo DiffusionCurveRenderer::RendererManager::Query(const QPoint& queryPoint)
{
    return mCurveSelectionRenderer->Query(queryPoint);
}
