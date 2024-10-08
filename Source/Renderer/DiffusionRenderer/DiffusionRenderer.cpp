#include "DiffusionRenderer.h"

#include "Core/Constants.h"
#include "Renderer/DiffusionRenderer/Renderers/BlurRenderer.h"
#include "Renderer/DiffusionRenderer/Renderers/ColorRenderer.h"
#include "Renderer/DiffusionRenderer/Renderers/DownsampleRenderer.h"
#include "Renderer/DiffusionRenderer/Renderers/UpsampleRenderer.h"
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

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::DiffusionRenderer::Render(QOpenGLFramebufferObject* target)
{
    mColorRenderer->Render(target);
    mDownsampleRenderer->Downsample(target);
    mUpsampleRenderer->Upsample(mDownsampleRenderer->GetFramebuffers());
    mBlurRenderer->Blur(target, mUpsampleRenderer->GetResult());
}

void DiffusionCurveRenderer::DiffusionRenderer::SetFramebufferSize(int newSize)
{
    mColorRenderer->SetFramebufferSize(newSize);
    mDownsampleRenderer->SetFramebufferSize(newSize);
    mUpsampleRenderer->SetFramebufferSize(newSize);
    mBlurRenderer->SetFramebufferSize(newSize);
}

void DiffusionCurveRenderer::DiffusionRenderer::SetSmoothIterations(int smoothIterations)
{
    mUpsampleRenderer->SetSmoothIterations(smoothIterations);
}

void DiffusionCurveRenderer::DiffusionRenderer::SetUseMultisampleFramebuffer(bool val)
{
    mColorRenderer->SetUseMultisampleFramebuffer(val);
}

int DiffusionCurveRenderer::DiffusionRenderer::GetSmoothIterations() const
{
    return mUpsampleRenderer->GetSmoothIterations();
}