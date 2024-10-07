#include "DiffusionRenderer.h"

#include "Core/Constants.h"
#include "Renderer/DiffusionRenderer/BlurRenderer.h"
#include "Renderer/DiffusionRenderer/ColorRenderer.h"
#include "Renderer/DiffusionRenderer/DownsampleRenderer.h"
#include "Renderer/DiffusionRenderer/UpsampleRenderer.h"
#include "Util/Chronometer.h"

void DiffusionCurveRenderer::DiffusionRenderer::Initialize()
{
    initializeOpenGLFunctions();

    mColorRenderer = new ColorRenderer;
    mColorRenderer->SetCamera(mCamera);
    mColorRenderer->SetCurveContainer(mCurveContainer);

    mDownsampleRenderer = new DownsampleRenderer;
    mUpsampleRenderer = new UpsampleRenderer;
    mBlurRenderer = new BlurRenderer;

    mBlurRenderer->SetCamera(mCamera);
    mBlurRenderer->SetCurveContainer(mCurveContainer);
}

void DiffusionCurveRenderer::DiffusionRenderer::Render(QOpenGLFramebufferObject* framebuffer)
{
    ClearFramebuffer(framebuffer);
    mColorRenderer->Render(framebuffer);
    mDownsampleRenderer->Downsample(framebuffer);
    mUpsampleRenderer->Upsample(mDownsampleRenderer->GetFramebuffers());
    mBlurRenderer->Blur(mUpsampleRenderer->GetResult());
}

void DiffusionCurveRenderer::DiffusionRenderer::ClearFramebuffer(QOpenGLFramebufferObject* framebuffer)
{
    framebuffer->bind();
    glViewport(0, 0, framebuffer->width(), framebuffer->height());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DiffusionCurveRenderer::DiffusionRenderer::SetFramebufferSize(int newSize)
{
    mFramebufferSize = newSize;

    mDownsampleRenderer->DeleteFramebuffers();
    mUpsampleRenderer->DeleteFramebuffers();
    mBlurRenderer->DeleteFramebuffer();

    mDownsampleRenderer->SetFramebufferSize(mFramebufferSize);
    mUpsampleRenderer->SetFramebufferSize(mFramebufferSize);
    mBlurRenderer->SetFramebufferSize(mFramebufferSize);

    mDownsampleRenderer->CreateFramebuffers();
    mUpsampleRenderer->CreateFramebuffers();
    mBlurRenderer->CreateFramebuffer();
}

void DiffusionCurveRenderer::DiffusionRenderer::SetSmoothIterations(int smoothIterations)
{
    mUpsampleRenderer->SetSmoothIterations(smoothIterations);
}

QOpenGLFramebufferObject* DiffusionCurveRenderer::DiffusionRenderer::GetResult() const
{
    return mBlurRenderer->GetResult();
}

int DiffusionCurveRenderer::DiffusionRenderer::GetSmoothIterations() const
{
    return mUpsampleRenderer->GetSmoothIterations();
}
