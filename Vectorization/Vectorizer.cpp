#include "Vectorizer.h"
#include "imgui.h"

#include <Dependencies/Eigen/SparseCore>

#include <QDebug>

Vectorizer::Vectorizer(QObject *parent)
    : QObject(parent)
    , mGaussianStack(nullptr)
    , mEdgeStack(nullptr)
    , mEdgeTracer(nullptr)
    , mPotrace(nullptr)
    , mCurveConstructor(nullptr)
    , mColorSampler(nullptr)
    , mCannyUpperThreshold(200.0f)
    , mCannyLowerThreshold(20.0f)
    , mSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mVectorizationStatus(VectorizationStatus::Ready)
    , mSelectedGaussianLayer(0)
    , mSelectedEdgeLayer(0)
    , mInit(false)

{
    mBitmapRenderer = BitmapRenderer::instance();
    mCurveManager = CurveManager::instance();

    // onVectorize runs in a seperate thread
    connect(this, &Vectorizer::vectorize, this, &Vectorizer::onVectorize, Qt::QueuedConnection);

    // onLoadDone runs in a seperate thread
    connect(this, &Vectorizer::loadDone, this, &Vectorizer::onLoadDone, Qt::QueuedConnection);
}

void Vectorizer::load(QString path)
{
    if (mGaussianStack)
        delete mGaussianStack;

    if (mEdgeStack)
        delete mEdgeStack;

    if (mEdgeTracer)
        delete mEdgeTracer;

    if (mPotrace)
        delete mPotrace;

    if (mCurveConstructor)
        delete mCurveConstructor;

    if (mColorSampler)
        delete mColorSampler;

    mSelectedGaussianLayer = 0;
    mSelectedEdgeLayer = 0;

    mOriginalImage = cv::imread(path.toStdString(), cv::IMREAD_COLOR);

    mSubWorkMode = SubWorkMode::ViewOriginalImage;
    mBitmapRenderer->setData(mOriginalImage, GL_BGR);

    emit loadDone();
}

void Vectorizer::onLoadDone()
{
    cv::Canny(mOriginalImage, mEdgeImage, mCannyUpperThreshold, mCannyLowerThreshold);

    mVectorizationStatus = VectorizationStatus::CreatingGaussianStack;
    mGaussianStack = new GaussianStack;
    mGaussianStack->run(mOriginalImage);

    mVectorizationStatus = VectorizationStatus::CreatingEdgeStack;
    mEdgeStack = new EdgeStack;
    mEdgeStack->run(mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);

    mVectorizationStatus = VectorizationStatus::Ready;
    mInit = true;
    mUpdateData = true;
}

void Vectorizer::onVectorize()
{
    qDebug() << "onVectorize thread:" << QThread::currentThreadId();

    // Tracing edges
    mVectorizationStatus = VectorizationStatus::TracingEdges;
    mEdgeTracer = new EdgeTracer;
    mEdgeTracer->run(mEdgeStack->layer(mSelectedEdgeLayer), 10);

    qInfo() << "Chains detected."
            << "Number of chains is:" << mEdgeTracer->chains().size();

    // Create polylines
    mVectorizationStatus = VectorizationStatus::CreatingPolylines;
    mPotrace = new Potrace;
    mPotrace->run(mEdgeTracer->chains());

    // Now construct curves using polylines
    mVectorizationStatus = VectorizationStatus::ConstructingCurves;
    mCurveConstructor = new CurveConstructor;
    mCurveConstructor->run(mPotrace->polylines());

    // Sampling Colors
    mColorSampler = new ColorSampler;
    cv::Mat imageLAB;
    cv::cvtColor(mOriginalImage, imageLAB, cv::COLOR_BGR2Lab);
    mColorSampler->run(mCurveConstructor->curves(), mOriginalImage, imageLAB, 0.05);

    // Set new curves
    mCurveManager->clear();
    mCurveManager->addCurves(mCurveConstructor->curves());

    mVectorizationStatus = VectorizationStatus::Finished;
    emit vectorizationDone();
}

void Vectorizer::draw()
{
    ImGui::Spacing();

    if (mVectorizationStatus == VectorizationStatus::CreatingGaussianStack)
    {
        if (mGaussianStack)
        {
            ImGui::Text("Status: Creating Gaussian Stack...");
            ImGui::ProgressBar(0.5f * mGaussianStack->progress());
        }

        return;

    } else if (mVectorizationStatus == VectorizationStatus::CreatingEdgeStack)
    {
        if (mEdgeStack)
        {
            ImGui::Text("Status: Creating Edge Stack...");
            ImGui::ProgressBar(0.5f + 0.5f * mEdgeStack->progress());
        }

        return;

    } else if (mVectorizationStatus == VectorizationStatus::TracingEdges)
    {
        if (mEdgeTracer)
        {
            ImGui::Text("Status: Tracing Edges...");
            ImGui::ProgressBar(0.45f * mEdgeTracer->progress());
        }

        return;

    } else if (mVectorizationStatus == VectorizationStatus::CreatingPolylines)
    {
        if (mPotrace)
        {
            ImGui::Text("Status: Creating Polylines...");
            ImGui::ProgressBar(0.45f + 0.45f * mPotrace->progress());
        }

        return;

    } else if (mVectorizationStatus == VectorizationStatus::ConstructingCurves)
    {
        if (mCurveConstructor)
        {
            ImGui::Text("Status: Constructing Curves...");
            ImGui::ProgressBar(0.9f + 0.1f * mCurveConstructor->progress());
        }

        return;
    }

    if (!mInit)
        return;

    if (mUpdateData)
    {
        mUpdateData = false;
        mSubWorkMode = SubWorkMode::ChooseEdgeStackLevel;
        mBitmapRenderer->setData(mEdgeStack->layer(mSelectedEdgeLayer), GL_RED);
    }

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sub Work Modes");

    int mode = (int) mSubWorkMode;

    if (ImGui::RadioButton("View Original Image##SubWorkMode", &mode, 0))
        mBitmapRenderer->setData(mOriginalImage, GL_BGR);

    if (ImGui::RadioButton("View Edges##SubWorkMode", &mode, 1))
        mBitmapRenderer->setData(mEdgeImage, GL_RED);

    if (ImGui::RadioButton("View Gaussian Stack##SubWorkMode", &mode, 2))
        mBitmapRenderer->setData(mGaussianStack->layer(mSelectedGaussianLayer), GL_BGR);

    if (ImGui::RadioButton("Choose Edge Stack Level##SubWorkMode", &mode, 3))
        mBitmapRenderer->setData(mEdgeStack->layer(mSelectedEdgeLayer), GL_RED);

    if (mSubWorkMode == SubWorkMode::ViewGaussianStack)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gaussian Stack Layers");

        if (ImGui::SliderInt("Layer##Gaussian", &mSelectedGaussianLayer, 0, mGaussianStack->height() - 1))
            mBitmapRenderer->setData(mGaussianStack->layer(mSelectedGaussianLayer), GL_BGR);
    }

    if (mSubWorkMode == SubWorkMode::ChooseEdgeStackLevel)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Edge Stack Layers");

        if (ImGui::SliderInt("Layer##Edge", &mSelectedEdgeLayer, 0, mEdgeStack->height() - 1))
            mBitmapRenderer->setData(mEdgeStack->layer(mSelectedEdgeLayer), GL_RED);

        if (ImGui::Button("Vectorize##Button"))
            emit vectorize();
    }

    mSubWorkMode = SubWorkMode(mode);
}

Vectorizer *Vectorizer::instance()
{
    static Vectorizer instance;
    return &instance;
}
