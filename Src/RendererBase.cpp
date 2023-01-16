#include "RendererBase.h"

DiffusionCurveRenderer::RendererBase::RendererBase()
    : mQualityFactor(1.0f)
    , mPixelRatio(1.0f)
    , mWidth(1600)
    , mHeight(900)
{}

void DiffusionCurveRenderer::RendererBase::Init()
{
    initializeOpenGLFunctions();
    mCurveManager = CurveManager::Instance();
    mShaderManager = ShaderManager::Instance();
    mCamera = EditModeCamera::Instance();
}

void DiffusionCurveRenderer::RendererBase::Resize(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

void DiffusionCurveRenderer::RendererBase::SetQualityFactor(float newQualityFactor)
{
    if (qFuzzyCompare(mQualityFactor, newQualityFactor))
        return;

    mQualityFactor = newQualityFactor;
}

void DiffusionCurveRenderer::RendererBase::SetPixelRatio(float newPixelRatio)
{
    if (qFuzzyCompare(mPixelRatio, newPixelRatio))
        return;

    mPixelRatio = newPixelRatio;
}

void DiffusionCurveRenderer::RendererBase::SetPoints(Points* newPoints)
{
    mPoints = newPoints;
}

void DiffusionCurveRenderer::RendererBase::SetQuad(Quad* newQuad)
{
    mQuad = newQuad;
}