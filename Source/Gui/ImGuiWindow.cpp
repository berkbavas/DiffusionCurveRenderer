#include "ImGuiWindow.h"

#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/RendererManager.h"
#include "Util/Chronometer.h"
#include "Util/Logger.h"

#include <QFileDialog>
#include <QtImGui.h>
#include <cmath>
#include <imgui.h>

DiffusionCurveRenderer::ImGuiWindow::ImGuiWindow(QObject* parent)
    : QObject(parent)
{

}

void DiffusionCurveRenderer::ImGuiWindow::Draw()
{
    mGlobalContourThickness = mCurveContainer->GetGlobalContourThickness();
    mGlobalDiffusionWidth = mCurveContainer->GetGlobalDiffusionWidth();
    mGlobalDiffusionGap = mCurveContainer->GetGlobalDiffusionGap();
    mSmoothIterations = mRendererManager->GetSmoothIterations();
    mFrambufferSize = mRendererManager->GetFramebufferSize();
    mFrambufferSizeIndex = std::log2(mFrambufferSize / 1024);

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_MenuBar);
    DrawMenuBar();
    DrawWorkModes();
    if (mWorkMode == WorkMode::Vectorization)
    {
        DrawVectorizationViewOptions();
    }
    else
    {
        DrawCurveEditingSettings();
    }
    ImGui::End();
    
    // Draw popups
    DrawAboutPopup();
    DrawShortcutsPopup();
}

void DiffusionCurveRenderer::ImGuiWindow::DrawWorkModes()
{
    if (ImGui::CollapsingHeader("Work Modes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int mode = (int) mWorkMode;
        ImGui::BeginDisabled(!mImageLoaded);
        ImGui::RadioButton("Vectorization##WorkMode", &mode, 0);
        ImGui::EndDisabled();
        ImGui::RadioButton("Curve Editing##WorkMode", &mode, 1);
        SetWorkMode(WorkMode(mode));
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawVectorizationViewOptions()
{
    if (mVectorizationStage == VectorizationStage::EdgeStack || mVectorizationStage == VectorizationStage::Finished)
    {
        if (ImGui::CollapsingHeader("Vectorization Options", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int option = static_cast<int>(mVectorizationViewOption);

            ImGui::RadioButton("View Original Image", &option, 0);
            ImGui::RadioButton("View Edges", &option, 1);
            ImGui::RadioButton("View Gaussian Stack", &option, 2);
            ImGui::RadioButton("Choose Edge Stack Level", &option, 3);

            SetVectorizationViewOption(VectorizationViewOption(option));

            if (mVectorizationViewOption == VectorizationViewOption::ViewGaussianStack)
            {
                ImGui::Text("Gaussian Stack Layers");

                if (ImGui::SliderInt("Layer##GaussianStack", &mGaussianStackLayer, 0, mMaximumGaussianStackLayer))
                {
                    emit GaussianStackLayerChanged(mGaussianStackLayer);
                }
            }

            if (mVectorizationViewOption == VectorizationViewOption::ChooseEdgeStackLevel)
            {
                ImGui::Text("Edge Stack Layers");

                if (ImGui::SliderInt("Layer##EdgeStach", &mEdgeStackLayer, 0, mMaximumEdgeStackLayer))
                {
                    emit EdgeStackLayerChanged(mEdgeStackLayer);
                }

                int option = static_cast<int>(mVectorizationCurveType);
                ImGui::Text("Choose curve type for vectorization:");
                ImGui::RadioButton("Bezier", &option, 0);
                ImGui::RadioButton("Spline", &option, 1);
                mVectorizationCurveType = VectorizationCurveType(option);

                if (ImGui::Button("Vectorize"))
                {
                    emit Vectorize(mVectorizationCurveType, mEdgeStackLayer);
                }
            }
        }
    }
    else
    {
        if (mVectorizationStage == VectorizationStage::GaussianStack)
        {
            ImGui::Text("Status: Creating Gaussian Stack...");
        }
        else if (mVectorizationStage == VectorizationStage::EdgeStack)
        {
            ImGui::Text("Status: Creating Edge Stack...");
        }
        else if (mVectorizationStage == VectorizationStage::EdgeTracer)
        {
            ImGui::Text("Status: Tracing Edges...");
        }
        else if (mVectorizationStage == VectorizationStage::Potrace)
        {
            ImGui::Text("Status: Creating Polylines...");
        }
        else if (mVectorizationStage == VectorizationStage::CurveContructor)
        {
            ImGui::Text("Status: Constructing Curves...");
        }
        else if (mVectorizationStage == VectorizationStage::ColorSampler)
        {
            ImGui::Text("Status: Sampling Colors...");
        }

        ImGui::ProgressBar(mVectorizationProgress);
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawCurveEditingSettings()
{
    DrawHintTexts();
    DrawRenderMode();
    DrawViewSettings();
    DrawCurveHeader();
    DrawRenderSettings();
    DrawStats();
}

void DiffusionCurveRenderer::ImGuiWindow::DrawMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open an image for vectorization", "Ctrl+O"))
            {
                QString path = QFileDialog::getOpenFileName(nullptr, "Select an image", "", "*.png *.jpg *.jpeg *.bmp");

                if (path.isNull() == false)
                {
                    qDebug() << "ImGuiWindow::DrawMenuBar(Select an image): Path is" << path;
                    AddRecentFile(path);
                    emit LoadImage(path);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Import XML", "Ctrl+I"))
            {
                QString path = QFileDialog::getOpenFileName(nullptr, "Select XML File", "", "*.xml");

                if (path.isNull() == false)
                {
                    qDebug() << "ImGuiWindow::DrawMenuBar(Import XML): Path is" << path;
                    AddRecentFile(path);
                    emit ImportXml(path);
                }
            }

            if (ImGui::MenuItem("Import JSON"))
            {
                QString path = QFileDialog::getOpenFileName(nullptr, "Select JSON File", "", "*.json");

                if (path.isNull() == false)
                {
                    qDebug() << "ImGuiWindow::DrawMenuBar(Import JSON): Path is" << path;
                    AddRecentFile(path);
                    emit ImportJson(path);
                }
            }

            ImGui::Separator();
            
            // Recent Files submenu
            if (ImGui::BeginMenu("Recent Files", !mRecentFiles.isEmpty()))
            {
                for (const QString& recentFile : mRecentFiles)
                {
                    if (ImGui::MenuItem(recentFile.toStdString().c_str()))
                    {
                        if (recentFile.endsWith(".xml", Qt::CaseInsensitive))
                        {
                            emit ImportXml(recentFile);
                        }
                        else if (recentFile.endsWith(".json", Qt::CaseInsensitive))
                        {
                            emit ImportJson(recentFile);
                        }
                        else
                        {
                            emit LoadImage(recentFile);
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Clear Recent Files"))
                {
                    mRecentFiles.clear();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save as PNG", "Ctrl+S"))
            {
                QString path = QFileDialog::getSaveFileName(nullptr, "PNG File", "", "*.png");

                if (path.isNull() == false)
                {
                    qDebug() << "ImGuiWindow::DrawMenuBar(Save as PNG): Path is" << path;
                    emit SaveAsPng(path);
                }
            }

            if (ImGui::MenuItem("Export as JSON", "Ctrl+E"))
            {
                QString path = QFileDialog::getSaveFileName(nullptr, "JSON File", "", "*.json");

                if (path.isNull() == false)
                {
                    qDebug() << "ImGuiWindow::DrawMenuBar(Export as JSON): Path is" << path;
                    emit ExportAsJson(path);
                }
            }

            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Duplicate Curve", "Ctrl+D", false, mSelectedCurve != nullptr))
            {
                emit DuplicateCurve(mSelectedCurve);
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Clear Canvas", "Ctrl+Shift+C"))
            {
                emit ClearCanvas();
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Reset View", "Home"))
            {
                emit ResetView();
            }
            
            if (ImGui::MenuItem("Zoom to Fit", "F"))
            {
                emit ZoomToFit();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Show Grid", nullptr, mShowGrid))
            {
                mShowGrid = !mShowGrid;
                emit ShowGridChanged(mShowGrid);
            }
            
            if (mShowGrid)
            {
                ImGui::Indent();
                if (ImGui::SliderFloat("Grid Spacing", &mGridSpacing, 10.0f, 200.0f))
                {
                    // Grid spacing updated
                }
                ImGui::Unindent();
            }
            
            ImGui::Separator();
            
            // Zoom display
            if (mCamera)
            {
                float zoomPercent = mCamera->GetZoom() * 100.0f;
                ImGui::Text("Zoom: %.0f%%", zoomPercent);
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Theme"))
        {
            if (ImGui::MenuItem("Dark", nullptr, mCurrentTheme == UITheme::Dark))
            {
                ApplyTheme(UITheme::Dark);
            }
            if (ImGui::MenuItem("Light", nullptr, mCurrentTheme == UITheme::Light))
            {
                ApplyTheme(UITheme::Light);
            }
            if (ImGui::MenuItem("Classic", nullptr, mCurrentTheme == UITheme::Classic))
            {
                ApplyTheme(UITheme::Classic);
            }
            
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Background Color", &mBackgroundColor[0]))
            {
                emit BackgroundColorChanged(mBackgroundColor);
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Keyboard Shortcuts", "F1"))
            {
                mShowShortcutsPopup = true;
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("About"))
            {
                mShowAboutPopup = true;
            }
            
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawHintTexts()
{
    if (ImGui::CollapsingHeader("Actions", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Selection         : Left Click");
        ImGui::Text("Add Control Point : Right Click");
        ImGui::Text("Add Color Point   : CTRL + Right Click");
        ImGui::Text("Move              : Middle button");
        ImGui::Text("Delete            : Delete key");
        ImGui::Text("Duplicate Curve   : CTRL + D");
        ImGui::Separator();
        if (ImGui::Checkbox("Show Color Point Handles", &mShowColorPointHandles))
        {
            emit ShowColorPointHandlesChanged(mShowColorPointHandles);
        }
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawRenderMode()
{
    if (ImGui::CollapsingHeader("Render Modes", ImGuiTreeNodeFlags_DefaultOpen))
    {
        mRenderModeContour = mRenderModes.testAnyFlag(RenderMode::Contour);
        mRenderModeDiffusion = mRenderModes.testAnyFlag(RenderMode::Diffusion);

        ImGui::Checkbox("Contour", &mRenderModeContour);
        ImGui::Checkbox("Diffusion", &mRenderModeDiffusion);

        SetRenderMode(RenderMode::Contour, mRenderModeContour);
        SetRenderMode(RenderMode::Diffusion, mRenderModeDiffusion);
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawCurveHeader()
{
    ImGui::BeginDisabled(!mSelectedCurve);
    if (ImGui::CollapsingHeader("Curve", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (mSelectedCurve)
        {
            if (std::dynamic_pointer_cast<Bezier>(mSelectedCurve))
            {
                ImGui::Text("Curve Type: Bezier");
            }
            else
            {
                ImGui::Text("Curve Type: B-Spline");
            }

            ImGui::Text("Number of Control Points: %d", mSelectedCurve->GetControlPoints().size());
            ImGui::Text("Curve Length: %.1f", mSelectedCurve->CalculateLength());
            ImGui::SliderFloat("Thickness", &mSelectedCurve->GetContourThickness_NonConst(), 1, 20);
            ImGui::SliderFloat("Diffusion Width", &mSelectedCurve->GetDiffusionWidth_NonConst(), 0.5f, 4.0f);
            ImGui::SliderFloat("Diffusion Gap", &mSelectedCurve->GetDiffusionGap_NonConst(), 0.5f, 4.0f);
            ImGui::ColorEdit4("Contour Color", &mSelectedCurve->GetContourColor_NonConst()[0]);

            ImGui::Spacing();
            
            if (ImGui::Button("Duplicate Curve"))
            {
                emit DuplicateCurve(mSelectedCurve);
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Curve"))
            {
                mCurveContainer->RemoveCurve(mSelectedCurve);
                SetSelectedCurve(nullptr);
            }
        }

        ImGui::Spacing();

        if (mSelectedControlPoint)
        {
            ImGui::Text("Control Point");
            if (ImGui::InputFloat2("Position (x,y)", &mSelectedControlPoint->position[0]))
                mSelectedCurve->Update();

            if (ImGui::Button("Remove Control Point"))
            {
                mSelectedCurve->RemoveControlPoint(mSelectedControlPoint);
                SetSelectedControlPoint(nullptr);
            }
        }

        ImGui::Spacing();

        if (mSelectedColorPoint)
        {
            ImGui::Text("Color Point");

            ImGui::Text("Direction: %s", mSelectedColorPoint->type == ColorPointType::Left ? "Left" : "Right");
            ImGui::SliderFloat("Position", &mSelectedColorPoint->position, 0.0f, 1.0f);

            if (ImGui::ColorEdit4("Color", &mSelectedColorPoint->color[0]))
            {
                mSelectedCurve->Update();
            }

            if (ImGui::Button("Remove Color Point"))
            {
                mSelectedCurve->RemoveColorPoint(mSelectedColorPoint);
                SetSelectedColorPoint(nullptr);
            }
        }
    }
    ImGui::EndDisabled();
}

void DiffusionCurveRenderer::ImGuiWindow::DrawRenderSettings()
{
    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::SliderInt("Smooth Iterations", &mSmoothIterations, 2, 50))
            mRendererManager->SetSmoothIterations(mSmoothIterations);

        if (ImGui::SliderInt("Frambuffer Size", &mFrambufferSizeIndex, 0, 2, FRAME_BUFFER_SIZES[mFrambufferSizeIndex]))
        {
            mFrambufferSize = 1024 * std::exp2(mFrambufferSizeIndex);
            mRendererManager->SetFramebufferSize(mFrambufferSize);
        }

        if (ImGui::SliderFloat("Global Thickness", &mGlobalContourThickness, 1.0f, 20.0f))
            mCurveContainer->SetGlobalContourThickness(mGlobalContourThickness);

        if (ImGui::SliderFloat("Global Diffusion Width", &mGlobalDiffusionWidth, 0.5f, 4.0f))
            mCurveContainer->SetGlobalDiffusionWidth(mGlobalDiffusionWidth);

        if (ImGui::SliderFloat("Global Diffusion Gap", &mGlobalDiffusionGap, 0.5f, 4.0f))
            mCurveContainer->SetGlobalDiffusionGap(mGlobalDiffusionGap);

        if (ImGui::Checkbox("Use Multisample Framebuffer", &mUseMultisampleFramebuffer))
            emit UseMultisampleFramebufferChanged(mUseMultisampleFramebuffer);

        if (ImGui::Button("Clear Canvas"))
        {
            emit ClearCanvas();
        }
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawStats()
{
    if (ImGui::CollapsingHeader("Stats"))
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        ImGui::Separator();
        ImGui::Text("Scene Statistics:");
        ImGui::Text("  Total Curves: %zu", mCurveContainer->GetTotalNumberOfCurves());
        
        // Calculate total control points
        size_t totalControlPoints = 0;
        size_t totalColorPoints = 0;
        for (int i = 0; i < mCurveContainer->GetTotalNumberOfCurves(); ++i)
        {
            auto curve = mCurveContainer->GetCurve(i);
            totalControlPoints += curve->GetNumberOfControlPoints();
        }
        ImGui::Text("  Total Control Points: %zu", totalControlPoints);
        
        if (mCamera)
        {
            ImGui::Separator();
            ImGui::Text("Camera:");
            ImGui::Text("  Zoom: %.0f%%", mCamera->GetZoom() * 100.0f);
            ImGui::Text("  Position: (%.1f, %.1f)", mCamera->GetLeft(), mCamera->GetTop());
        }
        
        ImGui::Separator();
        ImGui::Text("Performance Timings:");
        for (const auto& ID : ALL_CHORONOMETER_IDs)
            ImGui::Text("  %s", Chronometer::Print(ID).c_str());
    }
}

void DiffusionCurveRenderer::ImGuiWindow::SetSelectedCurve(CurvePtr selectedCurve)
{
    if (mSelectedCurve == selectedCurve)
        return;

    mSelectedCurve = selectedCurve;

    SetSelectedControlPoint(nullptr);
    SetSelectedColorPoint(nullptr);

    emit SelectedCurveChanged(selectedCurve);
}

void DiffusionCurveRenderer::ImGuiWindow::SetSelectedControlPoint(ControlPointPtr point)
{
    if (mSelectedControlPoint == point)
        return;

    mSelectedControlPoint = point;
    emit SelectedControlPointChanged(mSelectedControlPoint);
}

void DiffusionCurveRenderer::ImGuiWindow::SetSelectedColorPoint(ColorPointPtr point)
{
    if (mSelectedColorPoint == point)
        return;

    mSelectedColorPoint = point;
    emit SelectedColorPointChanged(mSelectedColorPoint);
}

void DiffusionCurveRenderer::ImGuiWindow::SetRenderMode(RenderMode mode, bool on)
{
    mRenderModes.setFlag(mode, on);
    emit RenderModesChanged(mRenderModes);
}

void DiffusionCurveRenderer::ImGuiWindow::SetWorkMode(WorkMode workMode)
{
    if (mWorkMode == workMode)
        return;

    mWorkMode = workMode;
    emit WorkModeChanged(mWorkMode);
}

void DiffusionCurveRenderer::ImGuiWindow::SetVectorizationStage(VectorizationStage state)
{
    mVectorizationStage = state;
}

void DiffusionCurveRenderer::ImGuiWindow::SetVectorizationViewOption(VectorizationViewOption option)
{
    if (mVectorizationViewOption == option)
        return;

    mVectorizationViewOption = option;

    emit VectorizationViewOptionChanged(mVectorizationViewOption);
}

void DiffusionCurveRenderer::ImGuiWindow::SetGaussianStackLayer(int layer)
{
    if (mGaussianStackLayer == layer)
        return;

    mGaussianStackLayer = layer;
    emit GaussianStackLayerChanged(mGaussianStackLayer);
}

void DiffusionCurveRenderer::ImGuiWindow::DrawViewSettings()
{
    if (ImGui::CollapsingHeader("View Settings"))
    {
        if (ImGui::Checkbox("Show Grid Overlay", &mShowGrid))
        {
            emit ShowGridChanged(mShowGrid);
        }
        
        if (mShowGrid)
        {
            ImGui::SliderFloat("Grid Spacing", &mGridSpacing, 10.0f, 200.0f);
        }
        
        if (ImGui::ColorEdit4("Background Color", &mBackgroundColor[0]))
        {
            emit BackgroundColorChanged(mBackgroundColor);
        }
        
        ImGui::Separator();
        
        if (mCamera)
        {
            ImGui::Text("Zoom: %.0f%%", mCamera->GetZoom() * 100.0f);
            
            if (ImGui::Button("Reset View"))
            {
                emit ResetView();
            }
            ImGui::SameLine();
            if (ImGui::Button("Zoom to Fit"))
            {
                emit ZoomToFit();
            }
        }
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawAboutPopup()
{
    if (mShowAboutPopup)
    {
        ImGui::OpenPopup("About");
        mShowAboutPopup = false;
    }
    
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Diffusion Curve Renderer");
        ImGui::Separator();
        ImGui::Text("Version 1.1.0");
        ImGui::Spacing();
        ImGui::Text("A C++ application for reconstructing images using");
        ImGui::Text("Bezier curves and color diffusion.");
        ImGui::Spacing();
        ImGui::Text("Based on the work of Orzan et al.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Technologies:");
        ImGui::BulletText("Qt 6 with OpenGL");
        ImGui::BulletText("ImGui for UI");
        ImGui::BulletText("OpenCV for image processing");
        ImGui::BulletText("Eigen for linear algebra");
        ImGui::Spacing();
        
        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void DiffusionCurveRenderer::ImGuiWindow::DrawShortcutsPopup()
{
    if (mShowShortcutsPopup)
    {
        ImGui::OpenPopup("Keyboard Shortcuts");
        mShowShortcutsPopup = false;
    }
    
    if (ImGui::BeginPopupModal("Keyboard Shortcuts", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Navigation");
        ImGui::Separator();
        ImGui::BulletText("Middle Mouse Button - Pan view");
        ImGui::BulletText("Mouse Wheel - Zoom in/out");
        ImGui::BulletText("Home - Reset view");
        ImGui::BulletText("F - Zoom to fit");
        ImGui::Spacing();
        
        ImGui::Text("Curve Editing");
        ImGui::Separator();
        ImGui::BulletText("Left Click - Select curve/point");
        ImGui::BulletText("Right Click - Add control point");
        ImGui::BulletText("Ctrl + Right Click - Add color point");
        ImGui::BulletText("Delete - Remove selected item");
        ImGui::BulletText("Ctrl + D - Duplicate curve");
        ImGui::Spacing();
        
        ImGui::Text("File Operations");
        ImGui::Separator();
        ImGui::BulletText("Ctrl + O - Open image");
        ImGui::BulletText("Ctrl + I - Import XML");
        ImGui::BulletText("Ctrl + S - Save as PNG");
        ImGui::BulletText("Ctrl + E - Export as JSON");
        ImGui::BulletText("Ctrl + Shift + C - Clear canvas");
        ImGui::Spacing();
        
        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void DiffusionCurveRenderer::ImGuiWindow::ApplyTheme(UITheme theme)
{
    mCurrentTheme = theme;
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    switch (theme)
    {
        case UITheme::Dark:
            ImGui::StyleColorsDark();
            style.FrameRounding = 4.0f;
            style.WindowRounding = 6.0f;
            style.GrabRounding = 4.0f;
            style.ScrollbarRounding = 6.0f;
            style.FrameBorderSize = 1.0f;
            
            // Customize dark theme colors
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.4f, 0.6f, 0.8f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.5f, 0.7f, 0.9f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.45f, 0.65f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.4f, 0.6f, 0.8f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.5f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.45f, 0.65f, 1.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.5f, 0.7f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.6f, 0.8f, 1.0f);
            break;
            
        case UITheme::Light:
            ImGui::StyleColorsLight();
            style.FrameRounding = 4.0f;
            style.WindowRounding = 6.0f;
            style.GrabRounding = 4.0f;
            style.ScrollbarRounding = 6.0f;
            style.FrameBorderSize = 1.0f;
            
            // Customize light theme
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.97f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.6f, 0.7f, 0.85f, 0.8f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.7f, 0.8f, 0.9f, 0.9f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.65f, 0.8f, 0.8f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.75f, 0.9f, 1.0f);
            break;
            
        case UITheme::Classic:
            ImGui::StyleColorsClassic();
            style.FrameRounding = 2.0f;
            style.WindowRounding = 4.0f;
            style.GrabRounding = 2.0f;
            break;
    }
    
    emit ThemeChanged(theme);
}

void DiffusionCurveRenderer::ImGuiWindow::AddRecentFile(const QString& path)
{
    // Remove if already exists
    mRecentFiles.removeAll(path);
    
    // Add to front
    mRecentFiles.prepend(path);
    
    // Keep only max number of recent files
    while (mRecentFiles.size() > MAX_RECENT_FILES)
    {
        mRecentFiles.removeLast();
    }
}
