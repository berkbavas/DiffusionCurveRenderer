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

void DiffusionCurveRenderer::RendererManager::Resize(int Width, int Height)
{
    mCurveSelectionRenderer->Resize(Width, Height);
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

void DiffusionCurveRenderer::RendererManager::RenderCurve(CurvePtr pCurve)
{
    mContourRenderer->RenderCurve(pCurve);
}

void DiffusionCurveRenderer::RendererManager::Save(const QString& Path, RenderModes RenderModes)
{
    mSaveFramebuffer = std::make_unique<QOpenGLFramebufferObject>(mCamera->GetWidth(), mCamera->GetHeight());

    // Clear
    glBindFramebuffer(GL_FRAMEBUFFER, mSaveFramebuffer->handle());
    glViewport(0, 0, mSaveFramebuffer->width(), mSaveFramebuffer->height());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (RenderModes.testAnyFlags(RenderMode::Diffusion))
    {
        mDiffusionRenderer->Render(mSaveFramebuffer.get());
    }

    if (RenderModes.testAnyFlags(RenderMode::Contour))
    {
        mContourRenderer->Render(mSaveFramebuffer.get());
    }

    mSaveFramebuffer->toImage().save(Path);
}

void DiffusionCurveRenderer::RendererManager::SetFramebufferSize(int Size)
{
    mFramebufferSize = Size;
    mDiffusionRenderer->SetFramebufferSize(Size);
}

void DiffusionCurveRenderer::RendererManager::SetSmoothIterations(int SmoothIterations)
{
    mDiffusionRenderer->SetSmoothIterations(SmoothIterations);
}

void DiffusionCurveRenderer::RendererManager::SetUseMultisampleFramebuffer(bool Value)
{
    mDiffusionRenderer->SetUseMultisampleFramebuffer(Value);
}

int DiffusionCurveRenderer::RendererManager::GetSmoothIterations() const
{
    return mDiffusionRenderer->GetSmoothIterations();
}

DiffusionCurveRenderer::CurveQueryInfo DiffusionCurveRenderer::RendererManager::Query(const QPoint& QueryPoint)
{
    return mCurveSelectionRenderer->Query(QueryPoint);
}
