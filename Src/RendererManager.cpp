#include "RendererManager.h"

#include <QImage>
#include <QMatrix4x4>

DiffusionCurveRenderer::RendererManager::RendererManager(QObject* parent)
    : Manager(parent)
    , mInitialFramebuffer(nullptr)
    , mFinalFramebuffer(nullptr)
    , mSmoothIterations(20)
    , mQualityFactor(1.0f)
    , mWidth(1600)
    , mHeight(900)
    , mPixelRatio(1.0f)
    , mSave(false)
{}

bool DiffusionCurveRenderer::RendererManager::Init()
{
    initializeOpenGLFunctions();
    // TODO

    mCurveManager = CurveManager::Instance();

    mPoints = new Points;
    mQuad = new Quad;

    mContourRenderer = new ContourRenderer;
    mContourRenderer->SetPoints(mPoints);
    mContourRenderer->SetQuad(mQuad);
    mContourRenderer->Init();

    mColorRenderer = new ColorRenderer;
    mColorRenderer->SetPoints(mPoints);
    mColorRenderer->SetQuad(mQuad);
    mColorRenderer->Init();

    mDiffusionRenderer = new DiffusionRenderer;
    mDiffusionRenderer->SetPoints(mPoints);
    mDiffusionRenderer->SetQuad(mQuad);
    mDiffusionRenderer->Init();

    mInitialFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mInitialFramebufferFormat.setSamples(0);
    mInitialFramebufferFormat.setMipmap(false);
    mInitialFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mInitialFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    mFinalFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFinalFramebufferFormat.setSamples(0);
    mFinalFramebufferFormat.setMipmap(false);
    mFinalFramebufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mFinalFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    mFinalMultisampleFramebufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFinalMultisampleFramebufferFormat.setSamples(16);
    mFinalMultisampleFramebufferFormat.setMipmap(false);
    mFinalMultisampleFramebufferFormat.setInternalTextureFormat(GL_RGBA8);

    // For color and blur textures
    mDrawBuffers = new GLenum[2];
    mDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    mDrawBuffers[1] = GL_COLOR_ATTACHMENT1;

    CreateFramebuffers();

    return true;
}

void DiffusionCurveRenderer::RendererManager::Render()
{
    if (mRenderMode == RenderMode::Diffusion)
    {
        mColorRenderer->Render(mInitialFramebuffer);
        mDiffusionRenderer->Render(mInitialFramebuffer, nullptr, true);

        // If a curve is selected then render it
        if (mCurveManager->GetSelectedCurve())
        {
            mContourRenderer->Render(nullptr, mCurveManager->GetSelectedCurve(), false);
        }
    }
    else if (mRenderMode == RenderMode::Contour)
    {
        mContourRenderer->Render(nullptr, true);
    }
    else if (mRenderMode == RenderMode::ContourAndDiffusion)
    {
        mColorRenderer->Render(mInitialFramebuffer);
        mDiffusionRenderer->Render(mInitialFramebuffer, nullptr, true);
        mContourRenderer->Render(nullptr, false);
    }

    if (mSave)
    {
        if (mRenderMode == RenderMode::Diffusion)
        {
            mColorRenderer->Render(mInitialFramebuffer);
            mDiffusionRenderer->Render(mInitialFramebuffer, mFinalFramebuffer, true);
            mFinalFramebuffer->toImage().save(mSavePath);
        }

        if (mRenderMode == RenderMode::Contour)
        {
            mContourRenderer->Render(mFinalMultisampleFramebuffer, true);
            mFinalMultisampleFramebuffer->toImage().save(mSavePath);
        }

        if (mRenderMode == RenderMode::ContourAndDiffusion)
        {
            mColorRenderer->Render(mInitialFramebuffer);
            mDiffusionRenderer->Render(mInitialFramebuffer, mFinalMultisampleFramebuffer, true);
            mContourRenderer->Render(mFinalMultisampleFramebuffer, false);
            mFinalMultisampleFramebuffer->toImage().save(mSavePath);
        }

        mSave = false;
    }
}

void DiffusionCurveRenderer::RendererManager::Resize(int width, int height)
{
    mWidth = width;
    mHeight = height;

    DeleteFramebuffers();
    CreateFramebuffers();

    mContourRenderer->Resize(mWidth, mHeight);
    mColorRenderer->Resize(mWidth, mHeight);
    mDiffusionRenderer->Resize(mWidth, mHeight);
}

DiffusionCurveRenderer::RendererManager* DiffusionCurveRenderer::RendererManager::Instance()
{
    static RendererManager instance;
    return &instance;
}

void DiffusionCurveRenderer::RendererManager::SetRenderMode(RenderMode newRenderMode)
{
    mRenderMode = newRenderMode;
}

void DiffusionCurveRenderer::RendererManager::CreateFramebuffers()
{
    int size = mQualityFactor * qMax(mWidth, mHeight);

    mInitialFramebuffer = new QOpenGLFramebufferObject(size, size, mInitialFramebufferFormat);
    mInitialFramebuffer->addColorAttachment(size, size); // For blur info
    mInitialFramebuffer->bind();
    glDrawBuffers(2, mDrawBuffers);
    mInitialFramebuffer->release();

    mFinalFramebuffer = new QOpenGLFramebufferObject(size, size, mFinalFramebufferFormat);
    mFinalMultisampleFramebuffer = new QOpenGLFramebufferObject(size, size, mFinalMultisampleFramebufferFormat);
}

void DiffusionCurveRenderer::RendererManager::DeleteFramebuffers()
{
    if (mInitialFramebuffer)
        delete mInitialFramebuffer;

    if (mFinalFramebuffer)
        delete mFinalFramebuffer;

    if (mFinalMultisampleFramebuffer)
        delete mFinalMultisampleFramebuffer;
}

void DiffusionCurveRenderer::RendererManager::SetQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;
    DeleteFramebuffers();
    CreateFramebuffers();

    mContourRenderer->SetQualityFactor(mQualityFactor);
    mColorRenderer->SetQualityFactor(mQualityFactor);
    mDiffusionRenderer->SetQualityFactor(mQualityFactor);
}

void DiffusionCurveRenderer::RendererManager::Save(const QString& path)
{
    mSave = true;
    mSavePath = path;
}

void DiffusionCurveRenderer::RendererManager::SetPixelRatio(float newPixelRatio)
{
    if (qFuzzyCompare(mPixelRatio, newPixelRatio))
        return;

    mPixelRatio = newPixelRatio;

    DeleteFramebuffers();
    CreateFramebuffers();

    mContourRenderer->SetPixelRatio(mPixelRatio);
    mColorRenderer->SetPixelRatio(mPixelRatio);
    mDiffusionRenderer->SetPixelRatio(mPixelRatio);
}

void DiffusionCurveRenderer::RendererManager::SetSmoothIterations(int newSmoothIterations)
{
    mSmoothIterations = newSmoothIterations;
    mDiffusionRenderer->SetSmoothIterations(mSmoothIterations);
}