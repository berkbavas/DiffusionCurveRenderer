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

#include <QtImGui.h>
#include <imgui.h>
#include <limits>

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
    connect(mEventHandler, &EventHandler::DuplicateCurveRequested, this, [this]() {
        DuplicateCurve(mSelectedCurve);
    });
    connect(mEventHandler, &EventHandler::ZoomToFitRequested, this, &Controller::OnZoomToFit);
    connect(mEventHandler, &EventHandler::ResetViewRequested, this, &Controller::OnResetView);

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
    connect(mImGuiWindow, &ImGuiWindow::ClearCanvas, this, &Controller::ClearCanvas);

    connect(mImGuiWindow, &ImGuiWindow::SaveAsPng, this, [=](const QString& path)
            {
                mRendererManager->Save(path, mRenderModes); //
            });
    
    // New signal connections for enhanced features
    connect(mImGuiWindow, &ImGuiWindow::DuplicateCurve, this, &Controller::DuplicateCurve);
    connect(mImGuiWindow, &ImGuiWindow::BackgroundColorChanged, this, &Controller::OnBackgroundColorChanged);
    connect(mImGuiWindow, &ImGuiWindow::ShowGridChanged, this, &Controller::OnShowGridChanged);
    connect(mImGuiWindow, &ImGuiWindow::ResetView, this, &Controller::OnResetView);
    connect(mImGuiWindow, &ImGuiWindow::ZoomToFit, this, &Controller::OnZoomToFit);

    connect(mImGuiWindow, &ImGuiWindow::ExportAsJson, this, [=](const QString& path)
            {
                Exporter::ExportAsJson(mCurveContainer->GetCurves(), path); //
            });

    connect(mImGuiWindow, &ImGuiWindow::ImportXml, this, [=](const QString& path)
            {
                const auto& curves = Importer::ImportFromXml(path);

                if (curves.isEmpty() == false)
                {
                    ClearCanvas();
                    mImGuiWindow->SetRenderMode(RenderMode::Diffusion, true);
                    mImGuiWindow->SetRenderMode(RenderMode::Contour, true);
                    mCurveContainer->AddCurves(curves);
                } //
            });

    connect(mImGuiWindow, &ImGuiWindow::ImportJson, this, [=](const QString& path)
            {
                const auto& curves = Importer::ImportFromJson(path);

                if (curves.isEmpty() == false)
                {
                    ClearCanvas();
                    mImGuiWindow->SetRenderMode(RenderMode::Diffusion, true);
                    mImGuiWindow->SetRenderMode(RenderMode::Contour, true);
                    mCurveContainer->AddCurves(curves);
                } //
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

void DiffusionCurveRenderer::Controller::ClearCanvas()
{
    OnSelectedCurveChanged(nullptr);
    mCurveContainer->Clear();
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
    ClearCanvas();
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

void DiffusionCurveRenderer::Controller::DuplicateCurve(CurvePtr curve)
{
    if (!curve)
        return;
    
    // Clone the curve with an offset
    CurvePtr clonedCurve = curve->Clone(QVector2D(30, 30));
    
    if (clonedCurve)
    {
        mCurveContainer->AddCurve(clonedCurve);
        OnSelectedCurveChanged(clonedCurve);
        qDebug() << "Controller::DuplicateCurve: Curve duplicated successfully";
    }
}

void DiffusionCurveRenderer::Controller::OnBackgroundColorChanged(const QVector4D& color)
{
    mBackgroundColor = color;
    mRendererManager->SetBackgroundColor(color);
}

void DiffusionCurveRenderer::Controller::OnShowGridChanged(bool show)
{
    mShowGrid = show;
    mOverlayPainter->SetShowGrid(show);
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
    
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    
    for (int i = 0; i < mCurveContainer->GetTotalNumberOfCurves(); ++i)
    {
        auto curve = mCurveContainer->GetCurve(i);
        for (int j = 0; j < curve->GetNumberOfControlPoints(); ++j)
        {
            QVector2D pos = curve->GetControlPointPosition(j);
            minX = std::min(minX, pos.x());
            minY = std::min(minY, pos.y());
            maxX = std::max(maxX, pos.x());
            maxY = std::max(maxY, pos.y());
        }
    }
    
    // Add padding
    float padding = 50.0f;
    minX -= padding;
    minY -= padding;
    maxX += padding;
    maxY += padding;
    
    // Set camera to fit the bounding box
    float width = maxX - minX;
    float height = maxY - minY;
    float aspectRatio = mWidth / mHeight;
    
    float zoom = 1.0f;
    if (width / height > aspectRatio)
    {
        zoom = mWidth / width;
    }
    else
    {
        zoom = mHeight / height;
    }
    
    mCamera->SetZoom(zoom);
    mCamera->SetLeft(minX);
    mCamera->SetTop(minY);
}
