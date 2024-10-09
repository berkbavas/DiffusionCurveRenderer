#include "DiffusionRenderer.h"

#include "Core/Constants.h"
#include "Renderer/DiffusionRenderer/Renderers/ColorRenderer.h"
#include "Renderer/DiffusionRenderer/Renderers/DownsampleRenderer.h"
#include "Renderer/DiffusionRenderer/Renderers/UpsampleRenderer.h"
#include "Util/Chronometer.h"

void DiffusionCurveRenderer::DiffusionRenderer::Initialize()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;

    mBlitter = new Shader("Blit Shader");
    mBlitter->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mBlitter->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Blit.frag");
    mBlitter->Initialize();

    mColorRenderer = new ColorRenderer;
    mColorRenderer->SetCamera(mCamera);
    mColorRenderer->SetCurveContainer(mCurveContainer);

    mDownsampleRenderer = new DownsampleRenderer;
    mUpsampleRenderer = new UpsampleRenderer;

    mFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFramebufferFormat.setSamples(0);

    SetFramebufferSize(DEFAULT_FRAMEBUFFER_SIZE);
}

void DiffusionCurveRenderer::DiffusionRenderer::Render(QOpenGLFramebufferObject* target)
{
    mColorRenderer->Render(mFramebuffer.get());
    mDownsampleRenderer->Downsample(mFramebuffer.get());
    mUpsampleRenderer->Upsample(mDownsampleRenderer->GetFramebuffers());

    if (target == nullptr)
    {
        // Blit auxilary framebuffer to the default frambuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, target->handle());
        glViewport(0, 0, target->width(), target->height());
    }

    mBlitter->Bind();
    mBlitter->SetSampler("sourceTexture", 0, mUpsampleRenderer->GetResult()->texture());
    mQuad->Render();
    mBlitter->Release();
}

void DiffusionCurveRenderer::DiffusionRenderer::SetFramebufferSize(int size)
{
    mFramebuffer = std::make_unique<QOpenGLFramebufferObject>(size, size, mFramebufferFormat);

    mColorRenderer->SetFramebufferSize(size);
    mDownsampleRenderer->SetFramebufferSize(size);
    mUpsampleRenderer->SetFramebufferSize(size);
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
