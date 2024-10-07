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
    , mCurveConstructor(this)
    , mColorSampler(this)
{
    Setup();
}

void DiffusionCurveRenderer::VectorizationManager::Setup()
{
    connect(&mGaussianStack, &VectorizationStageBase::Finished, this, [=]()
            { emit VectorizationStageFinished(VectorizationStage::GaussianStack, mGaussianStack.GetHeight() - 1); });

    connect(&mEdgeStack, &VectorizationStageBase::Finished, this, [=]()
            { emit VectorizationStageFinished(VectorizationStage::EdgeStack, mEdgeStack.GetHeight() - 1); });

    connect(&mEdgeTracer, &VectorizationStageBase::Finished, this, [=]()
            { emit VectorizationStageFinished(VectorizationStage::EdgeTracer); });

    connect(&mPotrace, &VectorizationStageBase::Finished, this, [=]()
            { emit VectorizationStageFinished(VectorizationStage::Potrace); });

    connect(&mCurveConstructor, &VectorizationStageBase::Finished, this, [=]()
            { emit VectorizationStageFinished(VectorizationStage::CurveContructor); });

    connect(&mColorSampler, &VectorizationStageBase::Finished, this, [=]()
            { emit VectorizationStageFinished(VectorizationStage::ColorSampler); });

    connect(&mGaussianStack, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.5f * fraction); });

    connect(&mEdgeStack, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.5f + 0.5f * fraction); });

    connect(&mEdgeTracer, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.40f * fraction); });

    connect(&mPotrace, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.40f + 0.40f * fraction); });

    connect(&mCurveConstructor, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.80f + 0.10f * fraction); });

    connect(&mColorSampler, &VectorizationStageBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.90f + 0.10f * fraction); });
}

void DiffusionCurveRenderer::VectorizationManager::Reset()
{
    mGaussianStack.Reset();
    mEdgeStack.Reset();
    mEdgeTracer.Reset();
    mPotrace.Reset();
    mCurveConstructor.Reset();
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

    SetVectorizationStage(VectorizationStage::EdgeStack);
    mEdgeStack.Run(&mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);
}

void DiffusionCurveRenderer::VectorizationManager::Vectorize(int edgeLevel)
{
    qDebug() << "VectorizationManager::LoadImage: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::LoadImage: Chosen Edge Level:" << edgeLevel;

    // Trace edges
    SetVectorizationStage(VectorizationStage::EdgeTracer);
    mEdgeTracer.Run(mEdgeStack.GetLayer(edgeLevel), 10);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mEdgeTracer.GetChains().size();

    // Create polylines
    SetVectorizationStage(VectorizationStage::Potrace);
    mPotrace.Run(mEdgeTracer.GetChains());
    qInfo() << "Number of polylines is:" << mPotrace.GetPolylines().size();

    SetVectorizationStage(VectorizationStage::CurveContructor);
    mCurveConstructor.Run(mPotrace.GetPolylines());

    SetVectorizationStage(VectorizationStage::ColorSampler);
    cv::Mat imageLAB;
    cv::cvtColor(mOriginalImage, imageLAB, cv::COLOR_BGR2Lab);
    mColorSampler.Run(mCurveConstructor.GetCurves(), mOriginalImage, imageLAB, 0.05);

    SetVectorizationStage(VectorizationStage::Finished);

    emit VectorizationFinished(mCurveConstructor.GetCurves());
}

void DiffusionCurveRenderer::VectorizationManager::SetVectorizationStage(VectorizationStage stage)
{
    if (mVectorizationStage == stage)
        return;

    mVectorizationStage = stage;
    emit VectorizationStageChanged(mVectorizationStage);
}
