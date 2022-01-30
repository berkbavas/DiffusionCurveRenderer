#include "RendererManager.h"
#include <QImage>

RendererManager::RendererManager()
    : mContourRenderer(nullptr)
    , mDownsampleRenderer(nullptr)
    , mUpsampleRenderer(nullptr)
    , mJacobiRenderer(nullptr)
    , mScreenRenderer(nullptr)
    , mRenderMode(RenderMode::Contours + RenderMode::Diffuse)
    , mColorRendererMode(ColorRendererMode::TriangleStrip)
    , mRenderQuality(RenderQuality::Low)
    , mBufferSize(2048.0f)
    , mQuality(1)
    , mInit(false)
    , mRenderQualityChanged(false)
    , mDiffusionWidth(2.0f)
    , mSmoothIterations(40)

{}

RendererManager::~RendererManager()
{
    if (mContourRenderer)
        delete mContourRenderer;
    if (mDownsampleRenderer)
        delete mDownsampleRenderer;
    if (mUpsampleRenderer)
        delete mUpsampleRenderer;
    if (mJacobiRenderer)
        delete mJacobiRenderer;
    if (mScreenRenderer)
        delete mScreenRenderer;

    mContourRenderer = nullptr;
    mDownsampleRenderer = nullptr;
    mUpsampleRenderer = nullptr;
    mJacobiRenderer = nullptr;
    mScreenRenderer = nullptr;

    deleteFramebuffers();
}

bool RendererManager::init()
{
    initializeOpenGLFunctions();

    // Define framebuffer format
    mFrambufferFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    mFrambufferFormat.setSamples(0);
    mFrambufferFormat.setMipmap(false);
    mFrambufferFormat.setTextureTarget(GL_TEXTURE_2D);
    mFrambufferFormat.setInternalTextureFormat(GL_RGBA8);

    createFramebuffers();

    mContourRenderer = new ContourRenderer;
    mColorRenderer = new ColorRenderer;
    mDownsampleRenderer = new DownsampleRenderer;
    mUpsampleRenderer = new UpsampleRenderer;
    mJacobiRenderer = new JacobiRenderer;
    mScreenRenderer = new ScreenRenderer;

    mInit = mContourRenderer->init();
    mInit &= mColorRenderer->init();
    mInit &= mDownsampleRenderer->init();
    mInit &= mUpsampleRenderer->init();
    mInit &= mJacobiRenderer->init();
    mInit &= mScreenRenderer->init();

    return mInit;
}

void RendererManager::render()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mRenderMode & RenderMode::Diffuse) {
        diffuse();
    }

    if (mRenderMode & RenderMode::Contours) {
        contours();
    }
}

void RendererManager::diffuse()
{
    if (mRenderQualityChanged) {
        mRenderQualityChanged = false;
        deleteFramebuffers();
        createFramebuffers();
    }

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();

    projectionMatrix.ortho(mQuality * mProjectionParameters->left,
                           mQuality * mProjectionParameters->left + mQuality * mProjectionParameters->width * mProjectionParameters->zoomRatio,
                           mQuality * mProjectionParameters->top + mQuality * mProjectionParameters->height * mProjectionParameters->zoomRatio,
                           mQuality * mProjectionParameters->top,
                           -1,
                           1);

    // Render color curves into initial framebuffer
    {
        mInitialFrameBuffer->bind();
        glViewport(0, 0, mInitialFrameBuffer->width(), mInitialFrameBuffer->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        ColorRenderer::Parameters parameters;
        parameters.curves = mCurveContainer->getCurves();
        parameters.projectionMatrix = projectionMatrix;
        parameters.mode = mColorRendererMode;
        parameters.gap = mDiffusionWidth;
        parameters.width = mDiffusionWidth;
        parameters.quality = mQuality;

        mColorRenderer->render(parameters);
        mInitialFrameBuffer->release();
    }

    // First downsample
    {
        mDownsampledFramebuffers[0]->bind();
        glViewport(0, 0, mDownsampledFramebuffers[0]->width(), mDownsampledFramebuffers[0]->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        mDownsampleRenderer->render(mInitialFrameBuffer->texture());
        mDownsampledFramebuffers[0]->release();
    }

    // Downsample 1,2,3...
    {
        for (int i = 1; i < mDownsampledFramebuffers.size(); ++i) {
            mDownsampledFramebuffers[i]->bind();
            glViewport(0, 0, mDownsampledFramebuffers[i]->width(), mDownsampledFramebuffers[i]->height());
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            mDownsampleRenderer->render(mDownsampledFramebuffers[i - 1]->texture());
            mDownsampledFramebuffers[i]->release();
        }
    }

    QOpenGLFramebufferObject::blitFramebuffer(mUpsampledFramebuffers.last(), mDownsampledFramebuffers.last());

    // Upsample and Smooth
    {
        int secondLast = mUpsampledFramebuffers.size() - 2;
        mUpsampledFramebuffers[secondLast]->bind();
        glViewport(0, 0, mUpsampledFramebuffers[secondLast]->width(), mUpsampledFramebuffers[secondLast]->height());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        mUpsampleRenderer->render(mUpsampledFramebuffers.last()->texture(), mDownsampledFramebuffers[secondLast]->texture());
        mJacobiRenderer->render(mDownsampledFramebuffers.last()->texture(), mUpsampledFramebuffers[secondLast]->texture());

        for (int j = 0; j < mSmoothIterations; j++) {
            QOpenGLFramebufferObject::blitFramebuffer(mTemporaryFrameBuffers.last(), mUpsampledFramebuffers.last());
            mJacobiRenderer->render(mDownsampledFramebuffers.last()->texture(), mUpsampledFramebuffers[secondLast]->texture());
        }

        mUpsampledFramebuffers[secondLast]->release();
    }

    // Upsample and Smooth
    {
        for (int i = mUpsampledFramebuffers.size() - 3; 0 <= i; --i) {
            mUpsampledFramebuffers[i]->bind();
            glViewport(0, 0, mUpsampledFramebuffers[i]->width(), mUpsampledFramebuffers[i]->height());
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            mUpsampleRenderer->render(mUpsampledFramebuffers[i + 1]->texture(), mDownsampledFramebuffers[i]->texture());

            for (int j = 0; j < mSmoothIterations; j++) {
                QOpenGLFramebufferObject::blitFramebuffer(mTemporaryFrameBuffers[i], mUpsampledFramebuffers[i]);
                mJacobiRenderer->render(mDownsampledFramebuffers[i]->texture(), mTemporaryFrameBuffers[i]->texture());
            }

            mUpsampledFramebuffers[i]->release();
        }
    }

    QOpenGLFramebufferObject::bindDefault();
    glViewport(0, 0, mProjectionParameters->width, mProjectionParameters->height);
    mScreenRenderer->render(mUpsampledFramebuffers[0]->texture(), mProjectionParameters->pixelRatio);
}

void RendererManager::contours()
{
    glViewport(0, 0, mProjectionParameters->width, mProjectionParameters->height);

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    projectionMatrix.ortho(mProjectionParameters->left, mProjectionParameters->right, mProjectionParameters->bottom, mProjectionParameters->top, -1, 1);

    mContourRenderer->render(mCurveContainer->getCurves(), projectionMatrix);
}

void RendererManager::setCurveContainer(const CurveContainer *newCurveContainer)
{
    mCurveContainer = newCurveContainer;
}

void RendererManager::setProjectionParameters(const ProjectionParameters *newProjectionParameters)
{
    mProjectionParameters = newProjectionParameters;
}

void RendererManager::onSmoothIterationsChanged(int smoothIterations)
{
    mSmoothIterations = smoothIterations;
}

void RendererManager::onDiffusionWidthChanged(float diffusionWidth)
{
    mDiffusionWidth = diffusionWidth;
}

void RendererManager::onContourThicknessChanged(float thickness)
{
    mContourRenderer->setContourThickness(thickness);
}

void RendererManager::onContourColorChanged(const QVector4D &color)
{
    mContourRenderer->setContourColor(color);
}

void RendererManager::onRenderQualityChanged(RenderQuality renderQuality)
{
    mRenderQualityChanged = true;
    mRenderQuality = renderQuality;
    switch (mRenderQuality) {
    case RenderQuality::Low:
        mQuality = 1;
        break;
    case RenderQuality::Fair:
        mQuality = 2;
        break;
    case RenderQuality::High:
        mQuality = 4;
        break;
    }
}

void RendererManager::onRenderModeChanged(RenderMode mode)
{
    mRenderMode = mode;
}

void RendererManager::onColorRendererModeChanged(ColorRendererMode mode)
{
    mColorRendererMode = mode;
}

void RendererManager::createFramebuffers()
{
    // Create initial framebuffer
    mInitialFrameBuffer = new QOpenGLFramebufferObject(mQuality * mBufferSize, mQuality * mBufferSize, mFrambufferFormat);

    // Create downsampled and upsampled buffers
    int bufferSize = mQuality * mBufferSize;
    while (bufferSize > 0) {
        mDownsampledFramebuffers << new QOpenGLFramebufferObject(bufferSize, bufferSize, mFrambufferFormat);
        mUpsampledFramebuffers << new QOpenGLFramebufferObject(bufferSize, bufferSize, mFrambufferFormat);
        mTemporaryFrameBuffers << new QOpenGLFramebufferObject(bufferSize, bufferSize, mFrambufferFormat);
        bufferSize = 0.5 * bufferSize;
    }
}

void RendererManager::deleteFramebuffers()
{
    for (int i = 0; i < mDownsampledFramebuffers.size(); i++) {
        if (mDownsampledFramebuffers[i]) {
            mDownsampledFramebuffers[i]->release();
            delete mDownsampledFramebuffers[i];
        }

        mDownsampledFramebuffers[i] = nullptr;
    }

    for (int i = 0; i < mUpsampledFramebuffers.size(); i++) {
        if (mUpsampledFramebuffers[i]) {
            mUpsampledFramebuffers[i]->release();
            delete mUpsampledFramebuffers[i];
        }
        mUpsampledFramebuffers[i] = nullptr;
    }

    for (int i = 0; i < mTemporaryFrameBuffers.size(); i++) {
        if (mTemporaryFrameBuffers[i]) {
            mTemporaryFrameBuffers[i]->release();
            delete mTemporaryFrameBuffers[i];
        }
        mTemporaryFrameBuffers[i] = nullptr;
    }

    if (mInitialFrameBuffer) {
        mInitialFrameBuffer->release();
        delete mInitialFrameBuffer;
    }

    mInitialFrameBuffer = nullptr;

    mDownsampledFramebuffers.clear();
    mUpsampledFramebuffers.clear();
    mTemporaryFrameBuffers.clear();
}
