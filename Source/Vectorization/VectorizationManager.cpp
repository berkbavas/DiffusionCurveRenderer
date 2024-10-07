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
{
    connect(&mGaussianStack, &VectorizationStateBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.5f * fraction); });

    connect(&mGaussianStack, &VectorizationStateBase::Finished, this, [=]()
            { emit GaussianStackFinished(mGaussianStack.GetHeight() - 1); });

    connect(&mEdgeStack, &VectorizationStateBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.5f + 0.5f * fraction); });

    connect(&mEdgeStack, &VectorizationStateBase::Finished, this, [=]()
            { emit EdgeStackFinished(mEdgeStack.GetHeight() - 1); });

    connect(&mEdgeTracer, &VectorizationStateBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.45f * fraction); });

    connect(&mPotrace, &VectorizationStateBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.45f + 0.45f * fraction); });

    connect(&mCurveConstructor, &VectorizationStateBase::ProgressChanged, this, [=](float fraction)
            { emit ProgressChanged(0.90f + 0.10f * fraction); });
}

void DiffusionCurveRenderer::VectorizationManager::Reset()
{
    mGaussianStack.Reset();
    mEdgeStack.Reset();
    mEdgeTracer.Reset();
    mPotrace.Reset();
    mCurveConstructor.Reset();
}

void DiffusionCurveRenderer::VectorizationManager::Prepare(const QString& path)
{
    Reset();

    mOriginalImage = cv::imread(path.toStdString(), cv::IMREAD_COLOR);

    emit ImageLoaded(mOriginalImage);

    cv::Canny(mOriginalImage, mCannyEdges, mCannyUpperThreshold, mCannyLowerThreshold);

    emit VectorizationStateChanged(VectorizationState::CreatingGaussianStack);
    mGaussianStack.Run(mOriginalImage);

    emit VectorizationStateChanged(VectorizationState::CreatingEdgeStack);
    mEdgeStack.Run(&mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);

    emit VectorizationStateChanged(VectorizationState::Ready);
}

void DiffusionCurveRenderer::VectorizationManager::LoadImage(const QString& path)
{
    qDebug() << "VectorizationManager::LoadImage: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::LoadImage: Path:" << path;
    Prepare(path);
}

void DiffusionCurveRenderer::VectorizationManager::Vectorize(int edgeLevel)
{
    qDebug() << "VectorizationManager::LoadImage: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::LoadImage: Chosen Edge Level:" << edgeLevel;

    // Trace edges
    emit VectorizationStateChanged(VectorizationState::TracingEdges);
    mEdgeTracer.Run(mEdgeStack.GetLayer(edgeLevel), 10);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mEdgeTracer.GetChains().size();

    // Create polylines
    emit VectorizationStateChanged(VectorizationState::CreatingPolylines);
    mPotrace.Run(mEdgeTracer.GetChains());
    qInfo() << "Number of polylines is:" << mPotrace.GetPolylines().size();

    emit VectorizationStateChanged(VectorizationState::ConstructingCurves);
    mCurveConstructor.Run(mPotrace.GetPolylines());

    cv::Mat imageLAB;
    cv::cvtColor(mOriginalImage, imageLAB, cv::COLOR_BGR2Lab);
    mColorSampler.Run(mCurveConstructor.GetCurves(), mOriginalImage, imageLAB, 0.05);

    emit VectorizationStateChanged(VectorizationState::Finished);

    emit VectorizationFinished(mCurveConstructor.GetCurves());
}

void DiffusionCurveRenderer::VectorizationManager::SetVectorizationState(VectorizationState state)
{
    mVectorizationState = state;
    emit VectorizationStateChanged(mVectorizationState);
}
