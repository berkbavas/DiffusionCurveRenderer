#include "RendererManager.h"

#include "Core/Constants.h"
#include "Renderer/BitmapRenderer/BitmapRenderer.h"
#include "Renderer/ContourRenderer/ContourRenderer.h"
#include "Renderer/DiffusionRenderer/DiffusionRenderer.h"
#include "Util/Chronometer.h"

void DiffusionCurveRenderer::RendererManager::Initialize()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;
    mBlitter = new Blitter;
    mBlitter->SetCamera(mCamera);

    mContourRenderer = new ContourRenderer;
    mContourRenderer->SetCamera(mCamera);
    mContourRenderer->SetCurveContainer(mCurveContainer);
    mContourRenderer->SetBlitter(mBlitter);
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

    mContourFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mContourFramebufferFormat.setSamples(0);

    mDiffusionFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mDiffusionFramebufferFormat.setSamples(0);

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
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DiffusionCurveRenderer::RendererManager::RenderDiffusion()
{
    mDiffusionRenderer->Render(mDiffusionFramebuffer.get());

    mBlitter->Blit(nullptr, mDiffusionFramebuffer.get(), true);
}

void DiffusionCurveRenderer::RendererManager::RenderContours()
{
    mContourRenderer->Render(nullptr);
}

void DiffusionCurveRenderer::RendererManager::RenderForCurveSelection()
{
    mCurveSelectionRenderer->Render();
}

void DiffusionCurveRenderer::RendererManager::RenderCurve(CurvePtr curve)
{
    mContourRenderer->RenderCurve(nullptr, curve);
}

void DiffusionCurveRenderer::RendererManager::SetFramebufferSize(int size)
{
    FramebufferSize = size;

    mContourFramebuffer = std::make_unique<QOpenGLFramebufferObject>(size, size, mContourFramebufferFormat);

    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    mDiffusionFramebuffer = std::make_unique<QOpenGLFramebufferObject>(size, size, mDiffusionFramebufferFormat);
    mDiffusionFramebuffer->addColorAttachment(size, size); // For blur
    mDiffusionFramebuffer->bind();
    glDrawBuffers(2, ATTACHMENTS);
    mDiffusionFramebuffer->release();

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
