#include "Vectorizer.h"
#include "imgui.h"

#include <Eigen/SparseCore>

#include <QDebug>

DiffusionCurveRenderer::Vectorizer::Vectorizer(QObject* parent)
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
    mBitmapRenderer = BitmapRenderer::Instance();
    mCurveManager = CurveManager::Instance();

    // OnVectorize runs in a seperate thread
    connect(this, &DiffusionCurveRenderer::Vectorizer::Vectorize, this, &DiffusionCurveRenderer::Vectorizer::OnVectorize, Qt::QueuedConnection);

    // OnLoadDone runs in a seperate thread
    connect(this, &DiffusionCurveRenderer::Vectorizer::LoadDone, this, &DiffusionCurveRenderer::Vectorizer::OnLoadDone, Qt::QueuedConnection);
}

void DiffusionCurveRenderer::Vectorizer::Load(QString path)
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
    mBitmapRenderer->SetData(mOriginalImage, GL_BGR);

    emit LoadDone();
}

void DiffusionCurveRenderer::Vectorizer::OnLoadDone()
{
    cv::Canny(mOriginalImage, mEdgeImage, mCannyUpperThreshold, mCannyLowerThreshold);

    mVectorizationStatus = VectorizationStatus::CreatingGaussianStack;
    mGaussianStack = new GaussianStack;
    mGaussianStack->Run(mOriginalImage);

    mVectorizationStatus = VectorizationStatus::CreatingEdgeStack;
    mEdgeStack = new EdgeStack;
    mEdgeStack->Run(mGaussianStack, mCannyLowerThreshold, mCannyUpperThreshold);

    mVectorizationStatus = VectorizationStatus::Ready;
    mInit = true;
    mUpdateData = true;
}

void DiffusionCurveRenderer::Vectorizer::OnVectorize()
{
    qDebug() << "onVectorize thread:" << QThread::currentThreadId();

    // Tracing edges
    mVectorizationStatus = VectorizationStatus::TracingEdges;
    mEdgeTracer = new EdgeTracer;
    mEdgeTracer->Run(mEdgeStack->GetLayer(mSelectedEdgeLayer), 10);

    qInfo() << "Chains detected."
        << "Number of chains is:" << mEdgeTracer->GetChains().size();

    // Create polylines
    mVectorizationStatus = VectorizationStatus::CreatingPolylines;
    mPotrace = new Potrace;
    mPotrace->Run(mEdgeTracer->GetChains());

    // Now construct curves using polylines
    mVectorizationStatus = VectorizationStatus::ConstructingCurves;
    mCurveConstructor = new CurveConstructor;
    mCurveConstructor->Run(mPotrace->GetPolylines());

    // Sampling Colors
    mColorSampler = new ColorSampler;
    cv::Mat imageLAB;
    cv::cvtColor(mOriginalImage, imageLAB, cv::COLOR_BGR2Lab);
    mColorSampler->Run(mCurveConstructor->GetCurves(), mOriginalImage, imageLAB, 0.05);

    // Set new curves
    mCurveManager->Clear();
    mCurveManager->AddCurves(mCurveConstructor->GetCurves());

    mVectorizationStatus = VectorizationStatus::Finished;
    emit VectorizationDone();
}

void DiffusionCurveRenderer::Vectorizer::Draw()
{
    ImGui::Spacing();

    if (mVectorizationStatus == VectorizationStatus::CreatingGaussianStack)
    {
        if (mGaussianStack)
        {
            ImGui::Text("Status: Creating Gaussian Stack...");
            ImGui::ProgressBar(0.5f * mGaussianStack->GetProgress());
        }

        return;
    }
    else if (mVectorizationStatus == VectorizationStatus::CreatingEdgeStack)
    {
        if (mEdgeStack)
        {
            ImGui::Text("Status: Creating Edge Stack...");
            ImGui::ProgressBar(0.5f + 0.5f * mEdgeStack->GetProgress());
        }

        return;
    }
    else if (mVectorizationStatus == VectorizationStatus::TracingEdges)
    {
        if (mEdgeTracer)
        {
            ImGui::Text("Status: Tracing Edges...");
            ImGui::ProgressBar(0.45f * mEdgeTracer->GetProgress());
        }

        return;
    }
    else if (mVectorizationStatus == VectorizationStatus::CreatingPolylines)
    {
        if (mPotrace)
        {
            ImGui::Text("Status: Creating Polylines...");
            ImGui::ProgressBar(0.45f + 0.45f * mPotrace->GetProgress());
        }

        return;
    }
    else if (mVectorizationStatus == VectorizationStatus::ConstructingCurves)
    {
        if (mCurveConstructor)
        {
            ImGui::Text("Status: Constructing Curves...");
            ImGui::ProgressBar(0.9f + 0.1f * mCurveConstructor->GetProgress());
        }

        return;
    }

    if (!mInit)
        return;

    if (mUpdateData)
    {
        mUpdateData = false;
        mSubWorkMode = SubWorkMode::ChooseEdgeStackLevel;
        mBitmapRenderer->SetData(mEdgeStack->GetLayer(mSelectedEdgeLayer), GL_RED);
    }

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sub Work Modes");

    int mode = (int)mSubWorkMode;

    if (ImGui::RadioButton("View Original Image##SubWorkMode", &mode, 0))
        mBitmapRenderer->SetData(mOriginalImage, GL_BGR);

    if (ImGui::RadioButton("View Edges##SubWorkMode", &mode, 1))
        mBitmapRenderer->SetData(mEdgeImage, GL_RED);

    if (ImGui::RadioButton("View Gaussian Stack##SubWorkMode", &mode, 2))
        mBitmapRenderer->SetData(mGaussianStack->GetLayer(mSelectedGaussianLayer), GL_BGR);

    if (ImGui::RadioButton("Choose Edge Stack Level##SubWorkMode", &mode, 3))
        mBitmapRenderer->SetData(mEdgeStack->GetLayer(mSelectedEdgeLayer), GL_RED);

    if (mSubWorkMode == SubWorkMode::ViewGaussianStack)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gaussian Stack Layers");

        if (ImGui::SliderInt("Layer##Gaussian", &mSelectedGaussianLayer, 0, mGaussianStack->GetHeight() - 1))
            mBitmapRenderer->SetData(mGaussianStack->GetLayer(mSelectedGaussianLayer), GL_BGR);
    }

    if (mSubWorkMode == SubWorkMode::ChooseEdgeStackLevel)
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Edge Stack Layers");

        if (ImGui::SliderInt("Layer##Edge", &mSelectedEdgeLayer, 0, mEdgeStack->GetHeight() - 1))
            mBitmapRenderer->SetData(mEdgeStack->GetLayer(mSelectedEdgeLayer), GL_RED);

        if (ImGui::Button("Vectorize##Button"))
            emit Vectorize();
    }

    mSubWorkMode = SubWorkMode(mode);
}

DiffusionCurveRenderer::Vectorizer* DiffusionCurveRenderer::Vectorizer::Instance()
{
    static Vectorizer instance;
    return &instance;
}