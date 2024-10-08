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
#include "Util/Importer.h"
#include "Util/Logger.h"
#include "Vectorization/VectorizationManager.h"

#include <QtImGui.h>
#include <imgui.h>

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

    connect(mImGuiWindow, &ImGuiWindow::SelectedCurveChanged, this, &Controller::OnSelectedCurveChanged);
    connect(mImGuiWindow, &ImGuiWindow::SelectedControlPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedControlPoint);

    connect(mImGuiWindow, &ImGuiWindow::SelectedCurveChanged, this, &Controller::OnSelectedCurveChanged);
    connect(mImGuiWindow, &ImGuiWindow::SelectedControlPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedControlPoint);
    connect(mImGuiWindow, &ImGuiWindow::SelectedColorPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedColorPoint);
    connect(mImGuiWindow, &ImGuiWindow::SelectedColorPointChanged, mOverlayPainter, &OverlayPainter::SetSelectedColorPoint);

    connect(mImGuiWindow, &ImGuiWindow::UseMultisampleFramebufferChanged, this, [=](bool val)
            { mRendererManager->SetUseMultisampleFramebuffer(val); });

    connect(mImGuiWindow, &ImGuiWindow::RenderModesChanged, this, [=](RenderModes renderModes)
            { mRenderModes = renderModes; });
    connect(mImGuiWindow, &ImGuiWindow::WorkModeChanged, this, [=](WorkMode workMode)
            { mWorkMode = workMode; });
    connect(mImGuiWindow, &ImGuiWindow::ShowColorPointHandlesChanged, this, [=](bool show)
            { mOverlayPainter->SetPaintColorPointHandles(show); });

    connect(mImGuiWindow, &ImGuiWindow::VectorizationViewOptionChanged, this, &Controller::OnVectorizationViewOptionChanged);
    connect(mImGuiWindow, &ImGuiWindow::GaussianStackLayerChanged, this, &Controller::OnGaussianStackLayerChanged);
    connect(mImGuiWindow, &ImGuiWindow::EdgeStackLayerChanged, this, &Controller::OnEdgeStackLayerChanged);

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

void DiffusionCurveRenderer::Controller::Render(float ifps)
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

void DiffusionCurveRenderer::Controller::OnKeyPressed(QKeyEvent* event)
{
    mEventHandler->OnKeyPressed(event);
}

void DiffusionCurveRenderer::Controller::OnKeyReleased(QKeyEvent* event)
{
    mEventHandler->OnKeyReleased(event);
}

void DiffusionCurveRenderer::Controller::Resize(int width, int height)
{
    mDevicePixelRatio = mWindow->devicePixelRatio();
    mWidth = width * mDevicePixelRatio;
    mHeight = height * mDevicePixelRatio;

    mWindow->makeCurrent();
    mCamera->Resize(mWidth, mHeight, mDevicePixelRatio);

    mRendererManager->Resize(mWidth, mHeight);
    mWindow->doneCurrent();
}

void DiffusionCurveRenderer::Controller::OnMousePressed(QMouseEvent* event)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mEventHandler->OnMousePressed(event);
}

void DiffusionCurveRenderer::Controller::OnMouseReleased(QMouseEvent* event)
{
    mEventHandler->OnMouseReleased(event);
}

void DiffusionCurveRenderer::Controller::OnMouseMoved(QMouseEvent* event)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mWindow->makeCurrent();
    mRendererManager->RenderForCurveSelection();
    mWindow->doneCurrent();

    mEventHandler->OnMouseMoved(event);
}

void DiffusionCurveRenderer::Controller::OnWheelMoved(QWheelEvent* event)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    mEventHandler->OnWheelMoved(event);
}

void DiffusionCurveRenderer::Controller::OnVectorizationViewOptionChanged(VectorizationViewOption option)
{
    qDebug() << (int) option;

    switch (option)
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
            int index = mImGuiWindow->GetGaussianStackLayer();
            cv::Mat image = mVectorizationManager->GetGaussianStackLayer(index);
            mBitmapRenderer->SetImage(image, GL_RGB8, GL_BGR);
            break;
        }
        case VectorizationViewOption::ChooseEdgeStackLevel:
        {
            int index = mImGuiWindow->GetEdgeStackLayer();
            cv::Mat image = mVectorizationManager->GetEdgeStackLayer(index);
            mBitmapRenderer->SetImage(image, GL_R8, GL_RED);
            break;
        }
        default:
            break;
    }
}

void DiffusionCurveRenderer::Controller::OnImageLoaded(cv::Mat image)
{
    mWindow->makeCurrent();
    mBitmapRenderer->SetImage(image, GL_RGB8, GL_BGR);
    mCamera->Reset();
    SetWorkMode(WorkMode::Vectorization);
    mImGuiWindow->SetVectorizationViewOption(VectorizationViewOption::ViewOriginalImage);
    mImGuiWindow->SetImageLoaded(true);
    mWindow->doneCurrent();
}

void DiffusionCurveRenderer::Controller::OnGaussianStackLayerChanged(int layer)
{
    cv::Mat image = mVectorizationManager->GetGaussianStackLayer(layer);
    mBitmapRenderer->SetImage(image, GL_RGB8, GL_BGR);
}

void DiffusionCurveRenderer::Controller::OnEdgeStackLayerChanged(int layer)
{
    cv::Mat image = mVectorizationManager->GetEdgeStackLayer(layer);
    mBitmapRenderer->SetImage(image, GL_R8, GL_RED);
}

void DiffusionCurveRenderer::Controller::OnVectorizationStageFinished(VectorizationStage stage, QVariant additionalData)
{
    switch (stage)
    {
        case VectorizationStage::GaussianStack:
            mImGuiWindow->SetMaximumGaussianStackLayer(additionalData.toInt());
            break;
        case VectorizationStage::EdgeStack:
            mImGuiWindow->SetMaximumEdgeStackLayer(additionalData.toInt());
            break;
        default:
            break;
    }
}

void DiffusionCurveRenderer::Controller::OnVectorizationFinished(const QVector<CurvePtr>& curves)
{
    OnSelectedCurveChanged(nullptr);
    mCurveContainer->Clear();
    mCurveContainer->AddCurves(curves);

    mImGuiWindow->SetRenderMode(RenderMode::Diffusion, true);
    mImGuiWindow->SetRenderMode(RenderMode::Contour, false);
    SetWorkMode(WorkMode::CurveEditing);
}

void DiffusionCurveRenderer::Controller::OnSelectedCurveChanged(CurvePtr selectedCurve)
{
    mSelectedCurve = selectedCurve;

    mOverlayPainter->SetSelectedCurve(mSelectedCurve);
    mImGuiWindow->SetSelectedCurve(mSelectedCurve);
    mEventHandler->SetSelectedCurve(mSelectedCurve);
}

void DiffusionCurveRenderer::Controller::SetWorkMode(WorkMode workMode)
{
    mWorkMode = workMode;
    mImGuiWindow->SetWorkMode(mWorkMode);
}
