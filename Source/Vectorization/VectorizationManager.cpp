#include "VectorizationManager.h"

#include <QThread>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

DiffusionCurveRenderer::VectorizationManager::VectorizationManager(QObject* parent)
    : QObject(parent)
    , mGaussianStack(this)
    , mEdgeStack(this)
    , mEdgeTracer(this)
    , mPotrace(this)
    , mSplineCurveConstructor(this)
    , mBezierCurveConstructor(this)
    , mColorSampler(this)
{
    Setup();
}

void DiffusionCurveRenderer::VectorizationManager::Setup()
{
    connect(&mGaussianStack, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.90f * fraction); });

    connect(&mEdgeStack, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.90f + 0.10f * fraction); });

    connect(&mEdgeTracer, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.40f * fraction); });

    connect(&mPotrace, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.40f + 0.20f * fraction); });

    connect(&mBezierCurveConstructor, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.60f + 0.20f * fraction); });

    connect(&mSplineCurveConstructor, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.60f + 0.20f * fraction); });

    connect(&mColorSampler, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.80f + 0.20f * fraction); });
}

void DiffusionCurveRenderer::VectorizationManager::Reset()
{
    mGaussianStack.Reset();
    mEdgeStack.Reset();
    mEdgeTracer.Reset();
    mPotrace.Reset();
    mSplineCurveConstructor.Reset();
    mBezierCurveConstructor.Reset();
    mColorSampler.Reset();
}

void DiffusionCurveRenderer::VectorizationManager::LoadImage(const QString& path)
{
    qDebug() << "VectorizationManager::LoadImage: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::LoadImage: Path:" << path;
    Prepare(path);
}

void DiffusionCurveRenderer::VectorizationManager::Prepare(const QString& path)
{
    Reset();

    SetVectorizationStage(VectorizationStage::Initial);

    mOriginalImage = cv::imread(path.toStdString(), cv::IMREAD_COLOR);

    emit ImageLoaded(mOriginalImage);

    cv::Canny(mOriginalImage, mCannyEdges, mCannyUpperThreshold, mCannyLowerThreshold);

    SetVectorizationStage(VectorizationStage::GaussianStack);
    mGaussianStack.Run(mOriginalImage);
    emit VectorizationStageFinished(VectorizationStage::GaussianStack, mGaussianStack.GetHeight() - 1);

    SetVectorizationStage(VectorizationStage::EdgeStack);
    mEdgeStack.Run(&mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);
    emit VectorizationStageFinished(VectorizationStage::EdgeStack, mEdgeStack.GetHeight() - 1);
}

void DiffusionCurveRenderer::VectorizationManager::Vectorize(VectorizationCurveType curveType, int edgeLevel)
{
    mEdgeTracer.Reset();
    mPotrace.Reset();
    mSplineCurveConstructor.Reset();
    mBezierCurveConstructor.Reset();
    mColorSampler.Reset();

    qDebug() << "VectorizationManager::LoadImage: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::LoadImage: Chosen Edge Level:" << edgeLevel;

    SetVectorizationStage(VectorizationStage::EdgeTracer);
    mEdgeTracer.Run(mEdgeStack.GetLayer(edgeLevel), 10);
    emit VectorizationStageFinished(VectorizationStage::EdgeTracer);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mEdgeTracer.GetChains().size();

    SetVectorizationStage(VectorizationStage::Potrace);
    mPotrace.Run(mEdgeTracer.GetChains());
    emit VectorizationStageFinished(VectorizationStage::Potrace);

    qInfo() << "Number of polylines is:" << mPotrace.GetPolylines().size();

    mCurrentCurveConstructor = nullptr;

    if (curveType == VectorizationCurveType::Bezier)
        mCurrentCurveConstructor = &mBezierCurveConstructor;
    else if (curveType == VectorizationCurveType::Spline)
        mCurrentCurveConstructor = &mSplineCurveConstructor;

    DCR_ASSERT(mCurrentCurveConstructor != nullptr);

    SetVectorizationStage(VectorizationStage::CurveContructor);
    mCurrentCurveConstructor->Run(mPotrace.GetPolylines());
    emit VectorizationStageFinished(VectorizationStage::CurveContructor);

    cv::Mat imageLAB;
    cv::cvtColor(mOriginalImage, imageLAB, cv::COLOR_BGR2Lab);

    SetVectorizationStage(VectorizationStage::ColorSampler);
    mColorSampler.Run(mCurrentCurveConstructor->GetCurves(), mOriginalImage, imageLAB, 0.05);
    emit VectorizationStageFinished(VectorizationStage::ColorSampler);

    SetVectorizationStage(VectorizationStage::Finished);

    emit VectorizationFinished(mCurrentCurveConstructor->GetCurves());
}

void DiffusionCurveRenderer::VectorizationManager::SetVectorizationStage(VectorizationStage stage)
{
    if (mVectorizationStage == stage)
        return;

    mVectorizationStage = stage;
    emit VectorizationStageChanged(mVectorizationStage);
}
