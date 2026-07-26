#include "VectorizationManager.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QImage>
#include <QTemporaryDir>
#include <QThread>

DiffusionCurveRenderer::VectorizationManager::VectorizationManager(QObject* pParent)
    : QObject(pParent)
    , mGaussianStack(nullptr)
    , mEdgeStack(nullptr)
    , mEdgeTracer(nullptr)
    , mPotrace(nullptr)
    , mSplineCurveConstructor(nullptr)
    , mBezierCurveConstructor(nullptr)
    , mColorSampler(nullptr)
{
    Setup();
}

void DiffusionCurveRenderer::VectorizationManager::Setup()
{
    connect(&mGaussianStack, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.90f * Fraction); });
    connect(&mEdgeStack, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.90f + 0.10f * Fraction); });
    connect(&mEdgeTracer, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.40f * Fraction); });
    connect(&mPotrace, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.40f + 0.20f * Fraction); });
    connect(&mBezierCurveConstructor, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.60f + 0.20f * Fraction); });
    connect(&mSplineCurveConstructor, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.60f + 0.20f * Fraction); });
    connect(&mColorSampler, &VectorizationStageBase::ProgressChanged, this, [=](float Fraction) { emit ProgressChanged(0.80f + 0.20f * Fraction); });
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

void DiffusionCurveRenderer::VectorizationManager::LoadImage(const QString& Path)
{
    qDebug() << "VectorizationManager::LoadImage: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::LoadImage: Path:" << Path;

    mOriginalImage = cv::imread(Path.toStdString(), cv::IMREAD_COLOR);

    emit ImageLoaded(mOriginalImage);

    Prepare();
}

void DiffusionCurveRenderer::VectorizationManager::Prepare()
{
    Reset();

    SetVectorizationStage(VectorizationStage::Initial);

    cv::Canny(mOriginalImage, mCannyEdges, mCannyUpperThreshold, mCannyLowerThreshold);

    SetVectorizationStage(VectorizationStage::GaussianStack);
    mGaussianStack.Run(mOriginalImage);
    emit VectorizationStageFinished(VectorizationStage::GaussianStack, mGaussianStack.GetHeight() - 1);

    SetVectorizationStage(VectorizationStage::EdgeStack);
    mEdgeStack.Run(&mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);
    emit VectorizationStageFinished(VectorizationStage::EdgeStack, mEdgeStack.GetHeight() - 1);
}

void DiffusionCurveRenderer::VectorizationManager::Vectorize(VectorizationCurveType CurveType, int EdgeLevel)
{
    mEdgeTracer.Reset();
    mPotrace.Reset();
    mSplineCurveConstructor.Reset();
    mBezierCurveConstructor.Reset();
    mColorSampler.Reset();

    qDebug() << "VectorizationManager::Vectorize: Current Thread: " << QThread::currentThread();
    qDebug() << "VectorizationManager::Vectorize: Chosen Edge Level:" << EdgeLevel;

    SetVectorizationStage(VectorizationStage::EdgeTracer);
    mEdgeTracer.Run(mEdgeStack.GetLayer(EdgeLevel), 10);
    emit VectorizationStageFinished(VectorizationStage::EdgeTracer);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mEdgeTracer.GetChains().size();

    SetVectorizationStage(VectorizationStage::Potrace);
    mPotrace.Run(mEdgeTracer.GetChains());
    emit VectorizationStageFinished(VectorizationStage::Potrace);

    qInfo() << "Number of polylines is:" << mPotrace.GetPolylines().size();

    mCurrentCurveConstructor = nullptr;

    if (CurveType == VectorizationCurveType::Bezier)
        mCurrentCurveConstructor = &mBezierCurveConstructor;
    else if (CurveType == VectorizationCurveType::Spline)
        mCurrentCurveConstructor = &mSplineCurveConstructor;

    DCR_ASSERT(mCurrentCurveConstructor != nullptr);

    SetVectorizationStage(VectorizationStage::CurveConstructor);
    mCurrentCurveConstructor->Run(mPotrace.GetPolylines());
    emit VectorizationStageFinished(VectorizationStage::CurveConstructor);

    cv::Mat ImageLab;
    cv::cvtColor(mOriginalImage, ImageLab, cv::COLOR_BGR2Lab);

    SetVectorizationStage(VectorizationStage::ColorSampler);
    mColorSampler.Run(mCurrentCurveConstructor->GetCurves(), mOriginalImage, ImageLab, 0.05);
    emit VectorizationStageFinished(VectorizationStage::ColorSampler);

    SetVectorizationStage(VectorizationStage::Finished);

    emit VectorizationFinished(mCurrentCurveConstructor->GetCurves());
}

void DiffusionCurveRenderer::VectorizationManager::SetVectorizationStage(VectorizationStage Stage)
{
    if (mVectorizationStage == Stage)
    {
        return;
    }

    mVectorizationStage = Stage;
    emit VectorizationStageChanged(mVectorizationStage);
}
