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

    mScreenShader = new Shader("Screen Shader");
    mScreenShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Quad.vert");
    mScreenShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Screen.frag");
    mScreenShader->Initialize();

    mContourRenderer = new ContourRenderer;
    mContourRenderer->SetCamera(mCamera);
    mContourRenderer->SetCurveContainer(mCurveContainer);

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
    mContourFramebufferFormat.setSamples(8);

    mDiffusionFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mDiffusionFramebufferFormat.setSamples(0);
    mDiffusionFramebufferFormat.setInternalTextureFormat(GL_RGBA8);
    mDiffusionFramebufferFormat.setMipmap(false);
    mDiffusionFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);

    CreateFramebuffers();
}

void DiffusionCurveRenderer::RendererManager::Resize(int width, int height)
{
    mCurveSelectionRenderer->Resize(width, height);
}

void DiffusionCurveRenderer::RendererManager::Clear()
{
    QOpenGLFramebufferObject::bindDefault();
    glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DiffusionCurveRenderer::RendererManager::RenderDiffusion()
{
    mDiffusionRenderer->Render(mDiffusionFramebuffer);

    BlitFromResult(mDiffusionRenderer->GetResult());
}

void DiffusionCurveRenderer::RendererManager::RenderContours(QVector4D* globalColorOption)
{
    mContourRenderer->Render(nullptr, globalColorOption);
}

void DiffusionCurveRenderer::RendererManager::RenderForCurveSelection()
{
    mCurveSelectionRenderer->Render();
}

void DiffusionCurveRenderer::RendererManager::RenderCurve(CurvePtr curve)
{
    mContourRenderer->RenderCurve(nullptr, curve);
}

void DiffusionCurveRenderer::RendererManager::BlitFromResult(QOpenGLFramebufferObject* result)
{
    QOpenGLFramebufferObject::bindDefault();
    glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    mScreenShader->Bind();
    mScreenShader->SetSampler("sourceTexture", 0, result->texture());
    mQuad->Render();
    mScreenShader->Release();
}

void DiffusionCurveRenderer::RendererManager::SetFramebufferSize(int newSize)
{
    mFramebufferSize = newSize;

    DeleteFramebuffers();
    CreateFramebuffers();

    mDiffusionRenderer->SetFramebufferSize(mFramebufferSize);
}

int DiffusionCurveRenderer::RendererManager::GetSmoothIterations()
{
    return mDiffusionRenderer->GetSmoothIterations();
}

void DiffusionCurveRenderer::RendererManager::SetSmoothIterations(int smoothIterations)
{
    mDiffusionRenderer->SetSmoothIterations(smoothIterations);
}

DiffusionCurveRenderer::CurveQueryInfo DiffusionCurveRenderer::RendererManager::Query(const QPoint& queryPoint)
{
    return mCurveSelectionRenderer->Query(queryPoint);
}

void DiffusionCurveRenderer::RendererManager::CreateFramebuffers()
{
    mContourFramebuffer = new QOpenGLFramebufferObject(mFramebufferSize, mFramebufferSize, mContourFramebufferFormat);

    constexpr GLuint ATTACHMENTS[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    mDiffusionFramebuffer = new QOpenGLFramebufferObject(mFramebufferSize, mFramebufferSize, mDiffusionFramebufferFormat);
    mDiffusionFramebuffer->addColorAttachment(mFramebufferSize, mFramebufferSize); // For blur
    mDiffusionFramebuffer->bind();
    glDrawBuffers(2, ATTACHMENTS);
    mDiffusionFramebuffer->release();
}

void DiffusionCurveRenderer::RendererManager::DeleteFramebuffers()
{
    if (mContourFramebuffer != nullptr)
    {
        delete mContourFramebuffer;
        mContourFramebuffer = nullptr;
    }

    if (mDiffusionFramebuffer != nullptr)
    {
        delete mDiffusionFramebuffer;
        mDiffusionFramebuffer = nullptr;
    }
}
