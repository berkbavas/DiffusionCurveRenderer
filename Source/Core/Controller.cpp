#include "Controller.h"

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Core/Window.h"
#include "EventHandler/EventHandler.h"
#include "Gui/ImGuiWindow.h"
#include "Gui/OverlayPainter.h"
#include "Renderer/BitmapRenderer/BitmapRenderer.h"
#include "Renderer/RendererManager.h"
#include "Util/Chronometer.h"
#include "Util/Exporter.h"
#include "Util/Importer.h"
#include "Util/Logger.h"
#include "Vectorization/VectorizationManager.h"

#include <imgui.h>
#include <limits>

#include <QtImGui.h>

DiffusionCurveRenderer::Controller::Controller(QObject* parent)
    : QObject(parent)
{
    mWindow = new Window;
    mImGuiWindow = new ImGuiWindow(this);
    mCamera = new OrthographicCamera;

    mOverlayPainter = new OverlayPainter(this);
    mOverlayPainter->SetDevice(mWindow);
    mOverlayPainter->SetCamera(mCamera);

    mCurveContainer = new CurveContainer;

    mRendererManager = new RendererManager;
    mRendererManager->SetCamera(mCamera);
    mRendererManager->SetCurveContainer(mCurveContainer);

    mEventHandler = new EventHandler(this);
    mEventHandler->SetCamera(mCamera);
    mEventHandler->SetCurveContainer(mCurveContainer);
    mEventHandler->SetRendererManager(mRendererManager);

    mImGuiWindow->SetCurveContainer(mCurveContainer);
    mImGuiWindow->SetRendererManager(mRendererManager);
    mImGuiWindow->SetCamera(mCamera);

    mVectorizationManager = new VectorizationManager;
    mVectorizationManagerThread = new QThread;
    mVectorizationManager->moveToThread(mVectorizationManagerThread);

    connect(mWindow, &Window::Initialize, this, &Controller::Initialize);
    connect(mWindow, &Window::Render, this, &Controller::Render);
    connect(mWindow, &Window::Resize, this, &Controller::Resize);
    connect(mWindow, &Window::MousePressed, this, &Controller::OnMousePressed);
    connect(mWindow, &Window::MouseReleased, this, &Controller::OnMouseReleased);
    connect(mWindow, &Window::MouseMoved, this, &Controller::OnMouseMoved);
    connect(mWindow, &Window::WheelMoved, this, &Controller::OnWheelMoved);
    connect(mWindow, &Window::KeyPressed, this, &Controller::OnKeyPressed);
    connect(mWindow, &Window::KeyReleased, this, &Controller::OnKeyReleased);

    connect(mEventHandler, &EventHandler::SelectedCurveChanged, this, &Controller::OnSelectedCurveChanged);
    connect(mEventHandler, &EventHandler::SelectedControlPointChanged, mImGuiWindow, &ImGuiWindow::SetSelectedControlPoint);
    connect(mEventHandler, &EventHandler::SelectedColorPointChanged, mImGuiWindow, &ImGuiWindow::SetSelectedColorPoint);
    connect(mEventHandler, &EventHandler::SelectedControlPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedControlPoint);
    connect(mEventHandler, &EventHandler::ControlPointAroundChanged, mOverlayPainter, &OverlayPainter::SetControlPointAround);
    connect(mEventHandler, &EventHandler::ColorPointAroundChanged, mOverlayPainter, &OverlayPainter::SetColorPointAround);
    connect(mEventHandler, &EventHandler::SelectedColorPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedColorPoint);

    // Connect keyboard shortcut signals from EventHandler
    connect(mEventHandler, &EventHandler::DuplicateCurveRequested, this, [this]() { DuplicateCurve(mSelectedCurve); });
    connect(mEventHandler, &EventHandler::ZoomToFitRequested, this, &Controller::OnZoomToFit);
    connect(mEventHandler, &EventHandler::ResetViewRequested, this, &Controller::OnResetView);

    connect(mImGuiWindow, &ImGuiWindow::SelectedCurveChanged, this, &Controller::OnSelectedCurveChanged);
    connect(mImGuiWindow, &ImGuiWindow::SelectedControlPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedControlPoint);

    connect(mImGuiWindow, &ImGuiWindow::SelectedCurveChanged, this, &Controller::OnSelectedCurveChanged);
    connect(mImGuiWindow, &ImGuiWindow::SelectedControlPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedControlPoint);
    connect(mImGuiWindow, &ImGuiWindow::SelectedColorPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedColorPoint);
    connect(mImGuiWindow, &ImGuiWindow::SelectedColorPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedColorPoint);

    connect(mImGuiWindow, &ImGuiWindow::UseMultisampleFramebufferChanged, this, [=](bool Val) { mRendererManager->SetUseMultisampleFramebuffer(Val); });

    connect(mImGuiWindow, &ImGuiWindow::RenderModesChanged, this, [=](RenderModes RenderModes) { mRenderModes = RenderModes; });
    connect(mImGuiWindow, &ImGuiWindow::WorkModeChanged, this, [=](WorkMode WorkMode) { mWorkMode = WorkMode; });
    connect(mImGuiWindow, &ImGuiWindow::ShowColorPointHandlesChanged, this, [=](bool Show) { mOverlayPainter->SetPaintColorPointHandles(Show); });

    connect(mImGuiWindow, &ImGuiWindow::VectorizationViewOptionChanged, this, &Controller::OnVectorizationViewOptionChanged);
    connect(mImGuiWindow, &ImGuiWindow::GaussianStackLayerChanged, this, &Controller::OnGaussianStackLayerChanged);
    connect(mImGuiWindow, &ImGuiWindow::EdgeStackLayerChanged, this, &Controller::OnEdgeStackLayerChanged);
    connect(mImGuiWindow, &ImGuiWindow::ClearCanvas, this, &Controller::ClearCanvas);

    connect(mImGuiWindow, &ImGuiWindow::SaveAsPng, this, [=](const QString& Path) { mRendererManager->Save(Path, mRenderModes); });

    // New signal connections for enhanced features
    connect(mImGuiWindow, &ImGuiWindow::DuplicateCurve, this, &Controller::DuplicateCurve);
    connect(mImGuiWindow, &ImGuiWindow::BackgroundColorChanged, this, &Controller::OnBackgroundColorChanged);
    connect(mImGuiWindow, &ImGuiWindow::ShowGridChanged, this, &Controller::OnShowGridChanged);
    connect(mImGuiWindow, &ImGuiWindow::ResetView, this, &Controller::OnResetView);
    connect(mImGuiWindow, &ImGuiWindow::ZoomToFit, this, &Controller::OnZoomToFit);

    connect(mImGuiWindow, &ImGuiWindow::ExportAsJson, this, [=](const QString& Path) { Exporter::ExportAsJson(mCurveContainer->GetCurves(), Path); });

    connect(mImGuiWindow, &ImGuiWindow::ImportXml, this, [=](const QString& Path) {
        const auto& Curves = Importer::ImportFromXml(Path);

        if (Curves.isEmpty() == false)
        {
            ClearCanvas();
            mImGuiWindow->SetRenderMode(RenderMode::Diffusion, true);
            mImGuiWindow->SetRenderMode(RenderMode::Contour, true);
            mCurveContainer->AddCurves(Curves);
        }
    });

    connect(mImGuiWindow, &ImGuiWindow::ImportJson, this, [=](const QString& Path) {
        const auto& Curves = Importer::ImportFromJson(Path);

        if (Curves.isEmpty() == false)
        {
            ClearCanvas();
            mImGuiWindow->SetRenderMode(RenderMode::Diffusion, true);
            mImGuiWindow->SetRenderMode(RenderMode::Contour, true);
            mCurveContainer->AddCurves(Curves);
        }
    });

    connect(mImGuiWindow, &ImGuiWindow::LoadImage, mVectorizationManager, &VectorizationManager::LoadImage, Qt::QueuedConnection);
    connect(mImGuiWindow, &ImGuiWindow::Vectorize, mVectorizationManager, &VectorizationManager::Vectorize, Qt::QueuedConnection);

    connect(mVectorizationManager, &VectorizationManager::ProgressChanged, mImGuiWindow, &ImGuiWindow::SetVectorizationProgress, Qt::QueuedConnection);
    connect(mVectorizationManager, &VectorizationManager::VectorizationStageChanged, mImGuiWindow, &ImGuiWindow::SetVectorizationStage, Qt::QueuedConnection);
    connect(mVectorizationManager, &VectorizationManager::ImageLoaded, this, &Controller::OnImageLoaded, Qt::QueuedConnection);
    connect(mVectorizationManager, &VectorizationManager::VectorizationStageFinished, this, &Controller::OnVectorizationStageFinished, Qt::QueuedConnection);
    connect(mVectorizationManager, &VectorizationManager::VectorizationFinished, this, &Controller::OnVectorizationFinished, Qt::QueuedConnection);
}

DiffusionCurveRenderer::Controller::~Controller()
{
    qDebug() << "Controller::~Controller: Application closing...";
    qDebug() << "Controller::~Controller: Current Thread:" << QThread::currentThread();

    mVectorizationManagerThread->quit();

    while (mVectorizationManagerThread->isRunning())
    {
        mVectorizationManagerThread->wait();
    }

    delete mVectorizationManagerThread;
}

void DiffusionCurveRenderer::Controller::Run()
{
    qInstallMessageHandler(Logger::QtMessageOutputCallback);

    qDebug() << "Controller::Controller: Application starting...";

    mVectorizationManagerThread->start();
    mWindow->resize(mWidth, mHeight);
    mWindow->show();
}

void DiffusionCurveRenderer::Controller::Initialize()
{
    initializeOpenGLFunctions();
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);

    mRendererManager->Initialize();
    mBitmapRenderer = mRendererManager->GetBitmapRenderer();

    QtImGui::initialize(mWindow);
}

void DiffusionCurveRenderer::Controller::Render(float Ifps)
{
    mDevicePixelRatio = mWindow->devicePixelRatio();
    mWidth = mWindow->width() * mDevicePixelRatio;
    mHeight = mWindow->height() * mDevicePixelRatio;

    { // RendererManager

        MEASURE_CALL_TIME(RENDERER_MANAGER);

        if (mWorkMode == WorkMode::CurveEditing)
        {
            mRendererManager->Clear();

            if (mRenderModes.testAnyFlag(RenderMode::Diffusion))
                mRendererManager->RenderDiffusion();

            if (mRenderModes.testAnyFlag(RenderMode::Contour))
                mRendererManager->RenderContours();

            if (mSelectedCurve)
                mRendererManager->RenderCurve(mSelectedCurve);
        }
        else if (mWorkMode == WorkMode::Vectorization)
        {
            mBitmapRenderer->Render();
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mWidth, mHeight);

    if (mWorkMode == WorkMode::CurveEditing)
    {
        mOverlayPainter->Render();
    }

    QtImGui::newFrame();

    mImGuiWindow->Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mWidth, mHeight);

    ImGui::Render();
    QtImGui::render();
}

void DiffusionCurveRenderer::Controller::OnKeyPressed(QKeyEvent* pEvent)
{
    mEventHandler->OnKeyPressed(pEvent);
}

void DiffusionCurveRenderer::Controller::OnKeyReleased(QKeyEvent* pEvent)
{
    mEventHandler->OnKeyReleased(pEvent);
}

void DiffusionCurveRenderer::Controller::Resize(int Width, int Height)
{
    mDevicePixelRatio = mWindow->devicePixelRatio();
    mWidth = Width * mDevicePixelRatio;
    mHeight = Height * mDevicePixelRatio;

    mWindow->makeCurrent();
    mCamera->Resize(mWidth, mHeight, mDevicePixelRatio);

    mRendererManager->Resize(mWidth, mHeight);
    mWindow->doneCurrent();
}

void DiffusionCurveRenderer::Controller::OnMousePressed(QMouseEvent* pEvent)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mEventHandler->OnMousePressed(pEvent);
}

void DiffusionCurveRenderer::Controller::OnMouseReleased(QMouseEvent* pEvent)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mEventHandler->OnMouseReleased(pEvent);
}

void DiffusionCurveRenderer::Controller::OnMouseMoved(QMouseEvent* pEvent)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mWindow->makeCurrent();
    mRendererManager->RenderForCurveSelection();
    mWindow->doneCurrent();

    mEventHandler->OnMouseMoved(pEvent);
}

void DiffusionCurveRenderer::Controller::OnWheelMoved(QWheelEvent* pEvent)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mEventHandler->OnWheelMoved(pEvent);
}

void DiffusionCurveRenderer::Controller::OnVectorizationViewOptionChanged(VectorizationViewOption Option)
{
    switch (Option)
    {
    case VectorizationViewOption::ViewOriginalImage:
    {
        cv::Mat image = mVectorizationManager->GetOriginalImage();
        mBitmapRenderer->SetImage(image, GL_RGB, GL_BGR);
        break;
    }
    case VectorizationViewOption::ViewEdges:
    {
        cv::Mat image = mVectorizationManager->GetCannyEdges();
        mBitmapRenderer->SetImage(image, GL_R8, GL_RED);
        break;
    }
    case VectorizationViewOption::ViewGaussianStack:
    {
        int Index = mImGuiWindow->GetGaussianStackLayer();
        cv::Mat Image = mVectorizationManager->GetGaussianStackLayer(Index);
        mBitmapRenderer->SetImage(Image, GL_RGB8, GL_BGR);
        break;
    }
    case VectorizationViewOption::ChooseEdgeStackLevel:
    {
        int Index = mImGuiWindow->GetEdgeStackLayer();
        cv::Mat Image = mVectorizationManager->GetEdgeStackLayer(Index);
        mBitmapRenderer->SetImage(Image, GL_R8, GL_RED);
        break;
    }
    default:
        break;
    }
}

void DiffusionCurveRenderer::Controller::OnImageLoaded(cv::Mat Image)
{
    mWindow->makeCurrent();
    mBitmapRenderer->SetImage(Image, GL_RGB8, GL_BGR);
    mCamera->Reset();
    SetWorkMode(WorkMode::Vectorization);
    mImGuiWindow->SetVectorizationViewOption(VectorizationViewOption::ViewOriginalImage);
    mImGuiWindow->SetImageLoaded(true);
    mWindow->doneCurrent();
}

void DiffusionCurveRenderer::Controller::ClearCanvas()
{
    OnSelectedCurveChanged(nullptr);
    mCurveContainer->Clear();
}

void DiffusionCurveRenderer::Controller::OnGaussianStackLayerChanged(int Layer)
{
    cv::Mat Image = mVectorizationManager->GetGaussianStackLayer(Layer);
    mBitmapRenderer->SetImage(Image, GL_RGB8, GL_BGR);
}

void DiffusionCurveRenderer::Controller::OnEdgeStackLayerChanged(int Layer)
{
    cv::Mat Image = mVectorizationManager->GetEdgeStackLayer(Layer);
    mBitmapRenderer->SetImage(Image, GL_R8, GL_RED);
}

void DiffusionCurveRenderer::Controller::OnVectorizationStageFinished(VectorizationStage Stage, QVariant AdditionalData)
{
    switch (Stage)
    {
    case VectorizationStage::GaussianStack:
        mImGuiWindow->SetMaximumGaussianStackLayer(AdditionalData.toInt());
        break;
    case VectorizationStage::EdgeStack:
        mImGuiWindow->SetMaximumEdgeStackLayer(AdditionalData.toInt());
        break;
    default:
        break;
    }
}

void DiffusionCurveRenderer::Controller::OnVectorizationFinished(const QVector<CurvePtr>& Curves)
{
    ClearCanvas();
    mCurveContainer->AddCurves(Curves);
    mImGuiWindow->SetRenderMode(RenderMode::Diffusion, true);
    mImGuiWindow->SetRenderMode(RenderMode::Contour, false);
    SetWorkMode(WorkMode::CurveEditing);
}

void DiffusionCurveRenderer::Controller::OnSelectedCurveChanged(CurvePtr SelectedCurve)
{
    mSelectedCurve = SelectedCurve;

    mOverlayPainter->SetSelectedCurve(mSelectedCurve);
    mImGuiWindow->SetSelectedCurve(mSelectedCurve);
    mEventHandler->SetSelectedCurve(mSelectedCurve);
}

void DiffusionCurveRenderer::Controller::SetWorkMode(WorkMode WorkMode)
{
    mWorkMode = WorkMode;
    mImGuiWindow->SetWorkMode(mWorkMode);
}

void DiffusionCurveRenderer::Controller::DuplicateCurve(CurvePtr Curve)
{
    if (!Curve)
    {
        return;
    }

    if (CurvePtr pClonedCurve = Curve->Clone(QVector2D(30, 30)))
    {
        mCurveContainer->AddCurve(pClonedCurve);
        OnSelectedCurveChanged(pClonedCurve);
    }
}

void DiffusionCurveRenderer::Controller::OnBackgroundColorChanged(const QVector4D& Color)
{
    mBackgroundColor = Color;
    mRendererManager->SetBackgroundColor(Color);
}

void DiffusionCurveRenderer::Controller::OnShowGridChanged(bool Show)
{
    mShowGrid = Show;
    mOverlayPainter->SetShowGrid(Show);
}

void DiffusionCurveRenderer::Controller::OnResetView()
{
    mCamera->Reset();
}

void DiffusionCurveRenderer::Controller::OnZoomToFit()
{
    // Calculate bounding box of all curves and fit the view
    if (mCurveContainer->GetTotalNumberOfCurves() == 0)
    {
        mCamera->Reset();
        return;
    }

    float MinX = std::numeric_limits<float>::max();
    float MinY = std::numeric_limits<float>::max();
    float MaxX = std::numeric_limits<float>::lowest();
    float MaxY = std::numeric_limits<float>::lowest();

    for (int i = 0; i < mCurveContainer->GetTotalNumberOfCurves(); ++i)
    {
        const auto pCurve = mCurveContainer->GetCurve(i);
        for (int j = 0; j < pCurve->GetNumberOfControlPoints(); ++j)
        {
            const auto Pos = pCurve->GetControlPointPosition(j);
            MinX = std::min(MinX, Pos.x());
            MinY = std::min(MinY, Pos.y());
            MaxX = std::max(MaxX, Pos.x());
            MaxY = std::max(MaxY, Pos.y());
        }
    }

    // Add padding
    float Padding = 50.0f;
    MinX -= Padding;
    MinY -= Padding;
    MaxX += Padding;
    MaxY += Padding;

    // Set camera to fit the bounding box
    float Width = MaxX - MinX;
    float Height = MaxY - MinY;
    float AspectRatio = mWidth / mHeight;
    float Zoom = 1.0f;

    if (Width / Height > AspectRatio)
    {
        Zoom = mWidth / Width;
    }
    else
    {
        Zoom = mHeight / Height;
    }

    mCamera->SetZoom(Zoom);
    mCamera->SetLeft(MinX);
    mCamera->SetTop(MinY);
}
