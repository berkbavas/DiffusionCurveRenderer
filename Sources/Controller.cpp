#include "Controller.h"
#include "Helper.h"
#include "Window.h"

#include <QApplication>
#include <QDebug>
#include <QOpenGLPaintDevice>
#include <QPaintDevice>
#include <QPainter>
#include <QtConcurrent>

DiffusionCurveRenderer::Controller::Controller(QObject* parent)
    : QObject(parent)
    , mIfps(0.0f)
    , mWorkMode(WorkMode::Edit)
    , mSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mPreviousSubWorkMode(SubWorkMode::ViewOriginalImage)
    , mActionMode(ActionMode::Select)
    , mRenderMode(RenderMode::Contour)
    , mGlobalContourThickness(DEFAULT_CONTOUR_THICKNESS)
    , mGlobalDiffusionWidth(DEFAULT_DIFFUSION_WIDTH)
    , mGlobalDiffusionGap(DEFAULT_DIFFUSION_GAP)
    , mGlobalContourColor(DEFAULT_CONTOUR_COLOR)
    , mGlobalBlurStrength(DEFAULT_BLUR_STRENGTH)
    , mSmoothIterations(20)
    , mQualityFactor(1)
    , mVoidThreshold(0.0f)
    , mVectorizerImageLoaded(false)
    , mShowFileDailog(false)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
    , mSelectedBlurPoint(nullptr)
{
    mDashedPen.setDashPattern({ 8, 8 });
    mDashedPen.setWidthF(1.0f);
    mDashedPen.setJoinStyle(Qt::MiterJoin);

    mDenseDashedPen.setDashPattern({ 4, 4 });
    mDenseDashedPen.setWidthF(1.0f);
    mDenseDashedPen.setJoinStyle(Qt::MiterJoin);

    mSolidPen.setWidthF(1.0f);
    mSolidPen.setJoinStyle(Qt::MiterJoin);
}

DiffusionCurveRenderer::Controller::~Controller()
{
    qDebug() << Q_FUNC_INFO;
}

void DiffusionCurveRenderer::Controller::Init()
{
    initializeOpenGLFunctions();

    mRendererManager = RendererManager::Instance();
    mCurveManager = CurveManager::Instance();
    mShaderManager = ShaderManager::Instance();
    mBitmapRenderer = BitmapRenderer::Instance();

    mVectorizer = Vectorizer::Instance();
    mVectorizer->moveToThread(&mVectorizerThread);
    connect(this, &DiffusionCurveRenderer::Controller::Draw, mVectorizer, &Vectorizer::Draw, Qt::DirectConnection);
    connect(this, &DiffusionCurveRenderer::Controller::Load, mVectorizer, &Vectorizer::Load, Qt::QueuedConnection);
    connect(mVectorizer, &Vectorizer::VectorizationDone, this, [=]()
        {
            mWorkMode = WorkMode::Edit;
            mActionMode = ActionMode::Select;
            mRenderMode = RenderMode::Contour;
        });

    mVectorizerThread.start();

    mEditModeCamera = EditModeCamera::Instance();
    mEditModeCamera->SetPixelRatio(mWindow->devicePixelRatio());

    mViewModeCamera = ViewModeCamera::Instance();
    mViewModeCamera->SetPixelRatio(mWindow->devicePixelRatio());

    mManagers << mRendererManager //
        << mCurveManager    //
        << mShaderManager   //
        << mBitmapRenderer;

    for (auto manager : mManagers)
        manager->Init();

    connect(mWindow, &Window::destroyed, this, [=]() { //
        qDebug() << Q_FUNC_INFO;
        if (mVectorizerThread.isRunning())
            mVectorizerThread.quit();
        });

    // QVector<Bezier *> curves = Helper::loadCurveDataFromXML(":Resources/CurveData/zephyr.xml");
    //    if (!curves.isEmpty())
    //    {
    //        mCurveManager->clear();
    //        mCurveManager->addCurves(curves);
    //    }
}

void DiffusionCurveRenderer::Controller::OnAction(Action action, CustomVariant value)
{
    switch (action)
    {
    case Action::Select: {
        mCurveManager->Select(mRenderMode, value.toVector2D(), mEditModeCamera->GetZoom() * 15.0f);
        break;
    }
    case Action::AddControlPoint: {
        mCurveManager->AddControlPoint(value.toVector2D());
        break;
    }
    case Action::AddColorPoint: {
        mCurveManager->AddColorPoint(value.toVector2D());
        break;
    }
    case Action::AddBlurPoint: {
        mCurveManager->AddBlurPoint(value.toVector2D());
        break;
    }
    case Action::RemoveCurve:
        mCurveManager->RemoveSelectedCurve();
        break;
    case Action::RemoveControlPoint:
        mCurveManager->RemoveSelectedControlPoint();
        break;
    case Action::RemoveColorPoint:
        mCurveManager->RemoveSelectedColorPoint();
        break;
    case Action::RemoveBlurPoint:
        mCurveManager->RemoveSelectedBlurPoint();
        break;
    case Action::UpdateControlPointPosition:
        if (mSelectedControlPoint)
            mSelectedControlPoint->mPosition = value.toVector2D();
        break;
    case Action::UpdateColorPointPosition:
        if (mSelectedColorPoint)
            mSelectedColorPoint->mPosition = mSelectedCurve->ParameterAt(value.toVector2D(), 10000);
        break;
    case Action::UpdateBlurPointPosition:
        if (mSelectedBlurPoint)
            mSelectedBlurPoint->mPosition = mSelectedCurve->ParameterAt(value.toVector2D(), 10000);
        break;
    case Action::ClearCanvas:
        mCurveManager->Clear();
        break;
    case Action::LoadFromXML: {
        QVector<Bezier*> curves = Helper::LoadCurveDataFromXML(value.toString());
        if (!curves.isEmpty())
        {
            mCurveManager->Clear();
            mCurveManager->AddCurves(curves);
        }
        break;
    }
    case Action::SaveAsPNG: {
        mRendererManager->Save(value.toString());
        break;
    }
    case Action::LoadFromJSON: {
        QVector<Bezier*> curves = Helper::LoadCurveDataFromJSON(value.toString());
        if (!curves.isEmpty())
        {
            mCurveManager->Clear();
            mCurveManager->AddCurves(curves);
        }
        break;
    }
    case Action::LoadVectorizerImage: {
        emit Load(value.toString());
        mVectorizerImageLoaded = true;
        mWorkMode = WorkMode::Vectorization;
        mSubWorkMode = SubWorkMode::ViewOriginalImage;
        break;
    }
    case Action::SaveAsJSON: {
        Helper::SaveCurveDataToJSON(mCurveManager->GetCurves(), value.toString());
        break;
    }
    case Action::ShowLoadFromJSONDialog:
        mFileDailogType = FileDialogType::LoadFromJSON;
        mShowFileDailog = true;
        break;
    case Action::ShowSaveAsJSONDialog:
        mFileDailogType = FileDialogType::SaveAsJSON;
        mShowFileDailog = true;
        break;
    case Action::ShowLoadFromXMLDialog:
        mFileDailogType = FileDialogType::LoadFromXML;
        mShowFileDailog = true;
        break;
    case Action::ShowSaveAsPNGDialog:
        mFileDailogType = FileDialogType::SaveAsPNG;
        mShowFileDailog = true;
        break;
    case Action::ShowLoadImageDialog:
        mFileDailogType = FileDialogType::LoadVectorizerImage;
        mShowFileDailog = true;
        break;
    }
}

void DiffusionCurveRenderer::Controller::Render(float ifps)
{
    mIfps = ifps;

    // Update member variables
    mSelectedCurve = mCurveManager->GetSelectedCurve();
    mSelectedControlPoint = mCurveManager->GetSelectedControlPoint();
    mSelectedColorPoint = mCurveManager->GetSelectedColorPoint();
    mSelectedBlurPoint = mCurveManager->GetSelectedBlurPoint();
    mPixelRatio = mWindow->devicePixelRatio();

    // Update member classes
    mEditModeCamera->SetPixelRatio(mPixelRatio);
    mViewModeCamera->SetPixelRatio(mPixelRatio);
    mBitmapRenderer->SetPixelRatio(mPixelRatio);
    mRendererManager->SetPixelRatio(mPixelRatio);
    mRendererManager->SetRenderMode(mRenderMode);

    if (mSelectedCurve)
    {
        mControlPoints = mSelectedCurve->GetControlPoints();
        mColorPoints = mSelectedCurve->GetAllColorPoints();
        mBlurPoints = mSelectedCurve->GetBlurPoints();
    }
    else
    {
        if (mActionMode == ActionMode::AddColorPoint)
            mActionMode = ActionMode::Select;

        mControlPoints.clear();
        mColorPoints.clear();
        mBlurPoints.clear();
    }

    // Render
    if (mWorkMode == WorkMode::Edit)
    {
        mEditModeCamera->Update(ifps);
        mRendererManager->Render();
    }
    else if (mWorkMode == WorkMode::Vectorization)
    {
        mViewModeCamera->Update(ifps);
        mBitmapRenderer->Render();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mPixelRatio * mWidth, mPixelRatio * mHeight);

    DrawPainter();
    DrawGUI();

    // ImGui Stuff
    mImGuiWantsMouseCapture = ImGui::GetIO().WantCaptureMouse;
    mImGuiWantsKeyboardCapture = ImGui::GetIO().WantCaptureKeyboard;

    // We must show file dialog in the render loop; otherwise the app crashes.
    if (mShowFileDailog)
    {
        mShowFileDailog = false;

        switch (mFileDailogType)
        {
        case FileDialogType::LoadFromXML: {
            QString path = QFileDialog::getOpenFileName(nullptr, "Select a XML file", QString(), "*.xml");
            if (!path.isNull())
                OnAction(Action::LoadFromXML, path);
            break;
        }
        case FileDialogType::LoadFromJSON: {
            QString path = QFileDialog::getOpenFileName(nullptr, "Select a JSON file", QString(), "*.json");
            if (!path.isNull())
                OnAction(Action::LoadFromJSON, path);
            break;
        }
        case FileDialogType::SaveAsPNG: {
            QString path = QFileDialog::getSaveFileName(nullptr, "Save as PNG", "", "*.png");
            if (!path.isNull())
                OnAction(Action::SaveAsPNG, path);
            break;
        }
        case FileDialogType::SaveAsJSON: {
            QString path = QFileDialog::getSaveFileName(nullptr, "Save as JSON", "", "*.json");
            if (!path.isNull())
                OnAction(Action::SaveAsJSON, path);
            break;
        }
        case FileDialogType::LoadVectorizerImage: {
            QString path = QFileDialog::getOpenFileName(nullptr, "Select an Image", "", "*.png *.jpg *.jpeg *.bmp");
            if (!path.isNull())
                OnAction(Action::LoadVectorizerImage, path);

            break;
        }
        }
    }
}

void DiffusionCurveRenderer::Controller::DrawGUI()
{
    QtImGui::newFrame();

    ImGui::SetNextWindowSize(ImVec2(420, 820), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", NULL, ImGuiWindowFlags_MenuBar);

    // Menu
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load an Image for Vectorization"))
                OnAction(Action::ShowLoadImageDialog);

            ImGui::Separator();

            if (ImGui::MenuItem("Load from XML"))
                OnAction(Action::ShowLoadFromXMLDialog);

            if (ImGui::MenuItem("Load from JSON"))
                OnAction(Action::ShowLoadFromJSONDialog);

            ImGui::Separator();

            if (ImGui::MenuItem("Save as PNG"))
                OnAction(Action::ShowSaveAsPNGDialog);

            if (ImGui::MenuItem("Save as JSON"))
                OnAction(Action::ShowSaveAsJSONDialog);

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Work Modes
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Work Modes");

        int mode = (int)mWorkMode;
        ImGui::BeginDisabled(!mVectorizerImageLoaded);
        ImGui::RadioButton("Vectorize##WorkMode", &mode, 0);
        ImGui::EndDisabled();
        ImGui::RadioButton("Edit##WorkMode", &mode, 1);
        mWorkMode = WorkMode(mode);
    }

    if (mWorkMode == WorkMode::Vectorization)
    {
        emit Draw();
    }
    else if (mWorkMode == WorkMode::Edit)
    {
        // Action Modes
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Actions Modes");

            int mode = (int)mActionMode;
            ImGui::RadioButton("Select##ActionMode", &mode, 0);
            ImGui::RadioButton("Add Control Point (Ctrl)##ActionMode", &mode, 1);
            ImGui::BeginDisabled(!mSelectedCurve);
            ImGui::RadioButton("Add Color Point (Alt)##ActionMode", &mode, 2);
            ImGui::RadioButton("Add Blur Point##ActionMode", &mode, 3);
            ImGui::EndDisabled();
            mActionMode = ActionMode(mode);
        }

        ImGui::Spacing();

        // Render Settings
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Render Settings");
            bool b0 = mRenderMode == RenderMode::Contour;
            bool b1 = mRenderMode == RenderMode::Diffusion;
            bool b2 = mRenderMode == RenderMode::ContourAndDiffusion;

            if (ImGui::Checkbox("Contours", &b0))
            {
                mRenderMode = RenderMode::Contour;
                mCurveManager->SetSelectedBlurPoint(nullptr);
                mCurveManager->SetSelectedColorPoint(nullptr);
            }

            if (ImGui::Checkbox("Diffusion", &b1))
                mRenderMode = RenderMode::Diffusion;

            if (ImGui::Checkbox("Contours and Diffusion", &b2))
                mRenderMode = RenderMode::ContourAndDiffusion;

            if (ImGui::SliderInt("Smooth Iterations", &mSmoothIterations, 2, 50))
                mRendererManager->SetSmoothIterations(mSmoothIterations);

            const char* names[3] = { "Empty", "Default", "High" };
            const char* elem_name = names[mQualityFactor];

            if (ImGui::SliderInt("Render Quality", &mQualityFactor, 1, 2, elem_name))
                mRendererManager->SetQualityFactor(mQualityFactor);

            if (ImGui::SliderFloat("Global Blur Strength", &mGlobalBlurStrength, 0.0f, 1.0f))
                mCurveManager->SetGlobalBlurStrength(mGlobalBlurStrength);

            if (ImGui::SliderFloat("Global Thickness", &mGlobalContourThickness, 1, 20))
                mCurveManager->SetGlobalContourThickness(mGlobalContourThickness);

            if (ImGui::SliderFloat("Global Diffusion Width", &mGlobalDiffusionWidth, 1, 20))
                mCurveManager->SetGlobalDiffusionWidth(mGlobalDiffusionWidth);

            if (ImGui::SliderFloat("Global Diffusion Gap", &mGlobalDiffusionGap, 0, 20))
                mCurveManager->SetGlobalDiffusionGap(mGlobalDiffusionGap);

            if (ImGui::ColorEdit4("Global Contour Color", &mGlobalContourColor[0]))
                mCurveManager->SetGlobalContourColor(mGlobalContourColor);

            if (ImGui::SliderFloat("Void Threshold", &mVoidThreshold, 0.0f, 32.0f))
                mCurveManager->MakeVoid(mVoidThreshold);
        }

        ImGui::Spacing();

        // Curve
        if (mSelectedCurve)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Curve");
            ImGui::Text("Number of Control Points: %d", (int)mSelectedCurve->GetControlPoints().size());
            ImGui::Text("Number of Color Points: %d", (int)mSelectedCurve->GetAllColorPoints().size());
            if (ImGui::InputInt("Depth", &mSelectedCurve->mDepth))
                mCurveManager->SortCurves();
            ImGui::SliderFloat("Thickness", &mSelectedCurve->mContourThickness, 1, 20);
            ImGui::SliderFloat("Diffusion Width", &mSelectedCurve->mDiffusionWidth, 1, 20);
            ImGui::SliderFloat("Diffusion Gap", &mSelectedCurve->mDiffusionGap, 0, 20);
            ImGui::ColorEdit4("Contour Color", &mSelectedCurve->mContourColor[0]);
            if (ImGui::Button("Remove Curve"))
                OnAction(Action::RemoveCurve);
        }

        ImGui::Spacing();

        // Control Point
        if (mSelectedControlPoint)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Control Point");
            ImGui::InputFloat2("Position (x,y)", &mSelectedControlPoint->mPosition[0]);
            if (ImGui::Button("Remove Control Point"))
                OnAction(Action::RemoveControlPoint);
        }

        ImGui::Spacing();

        // Color Point
        if (mSelectedColorPoint)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Color Point");

            ImGui::Text("Direction: %s", mSelectedColorPoint->mDirection == ColorPoint::Direction::Left ? "Left" : "Right");
            ImGui::SliderFloat("Position", &mSelectedColorPoint->mPosition, 0.0f, 1.0f);
            ImGui::ColorEdit4("Color", &mSelectedColorPoint->mColor[0]);

            if (ImGui::Button("Remove Color Point"))
                OnAction(Action::RemoveColorPoint);
        }

        ImGui::Spacing();

        // Blur Point
        if (mSelectedBlurPoint)
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Blur Point");

            ImGui::SliderFloat("Position", &mSelectedBlurPoint->mPosition, 0.0f, 1.0f);
            ImGui::SliderFloat("Strength", &mSelectedBlurPoint->mStrength, 0.0f, 1.0f);

            if (ImGui::Button("Remove Blur Point"))
                OnAction(Action::RemoveBlurPoint);
        }

        ImGui::Spacing();

        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Info");
        ImGui::Text("Number of Curves: %d", (int)mCurveManager->GetCurves().size());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (ImGui::Button("Clear Canvas"))
            OnAction(Action::ClearCanvas);
    }

    ImGui::End();

    ImGui::Render();
    QtImGui::render();
}

void DiffusionCurveRenderer::Controller::DrawPainter()
{
    if (mWorkMode == WorkMode::Edit && mSelectedCurve)
    {
        mDashedPen.setColor(QColor(0, 0, 0));
        mSolidPen.setColor(QColor(0, 0, 0));

        QOpenGLPaintDevice device(mPixelRatio * mWidth, mPixelRatio * mHeight);
        QPainter painter(&device);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // Control polygon
        painter.setPen(mDashedPen);
        painter.setBrush(QBrush());
        for (int i = 0; i < mControlPoints.size() - 1; ++i)
        {
            QPointF p0 = mEditModeCamera->ToGUI(mControlPoints[i]->mPosition);
            QPointF p1 = mEditModeCamera->ToGUI(mControlPoints[i + 1]->mPosition);
            painter.drawLine(p0, p1);
        }

        // Control Points

        for (int j = 0; j < mControlPoints.size(); ++j)
        {
            QPointF center = mEditModeCamera->ToGUI(mControlPoints[j]->mPosition);

            // Outer disk
            float outerRadius = mControlPoints[j]->mSelected ? 16 : 12;
            outerRadius = qMin(outerRadius, outerRadius / mEditModeCamera->GetZoom());
            painter.setBrush(QColor(128, 128, 128, 128));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(center, outerRadius, outerRadius);

            // Inner disk
            float innerRadius = 6;
            innerRadius = qMin(innerRadius, innerRadius / mEditModeCamera->GetZoom());
            painter.setBrush(QColor(255, 255, 255));
            painter.setPen(QColor(0, 0, 0, 0));
            painter.drawEllipse(center, innerRadius, innerRadius);
        }

        if (mRenderMode != RenderMode::Contour)
        {
            // Blur Points
            for (int i = 0; i < mBlurPoints.size(); ++i)
            {
                QPointF visualPosition = mEditModeCamera->ToGUI(mBlurPoints[i]->GetPosition2D(mEditModeCamera->GetZoom() * BLUR_POINT_VISUAL_GAP));
                QPointF actualPosition = mEditModeCamera->ToGUI(mSelectedCurve->ValueAt(mBlurPoints[i]->mPosition));

                // Draw a dashed line actual position to visual position
                painter.setPen(mDenseDashedPen);
                painter.drawLine(actualPosition, visualPosition);

                // Outer disk
                float outerRadius = mBlurPoints[i]->mSelected ? 36 : 24;
                outerRadius = qMin(outerRadius, outerRadius / mEditModeCamera->GetZoom());
                painter.setBrush(QColor(128, 128, 128, 128));
                painter.setPen(QColor(0, 0, 0, 0));
                painter.drawEllipse(visualPosition, outerRadius, outerRadius);

                // Inner disk
                float innerRadius = 8;
                innerRadius = qMin(innerRadius, innerRadius / mEditModeCamera->GetZoom());
                painter.setBrush(QColor(255, 255, 255));
                painter.setPen(QColor(0, 0, 0, 0));
                painter.drawEllipse(visualPosition, innerRadius, innerRadius);
            }

            // Color Points
            for (int i = 0; i < mColorPoints.size(); ++i)
            {
                QPointF visualPosition = mEditModeCamera->ToGUI(mColorPoints[i]->GetPosition2D(mEditModeCamera->GetZoom() * COLOR_POINT_VISUAL_GAP));
                QPointF actualPosition = mEditModeCamera->ToGUI(mSelectedCurve->ValueAt(mColorPoints[i]->mPosition));

                // Draw a dashed line actual position to visual position
                painter.setPen(mDenseDashedPen);
                painter.drawLine(actualPosition, visualPosition);

                // Outer disk
                float outerRadius = mColorPoints[i]->mSelected ? 16 : 12;
                outerRadius = qMin(outerRadius, outerRadius / mEditModeCamera->GetZoom());
                painter.setBrush(QColor(128, 128, 128, 128));
                painter.setPen(QColor(0, 0, 0, 0));
                painter.drawEllipse(visualPosition, outerRadius, outerRadius);

                // Inner disk
                float innerRadius = 6;
                innerRadius = qMin(innerRadius, innerRadius / mEditModeCamera->GetZoom());
                painter.setBrush(QColor(255 * mColorPoints[i]->mColor.x(), //
                    255 * mColorPoints[i]->mColor.y(),
                    255 * mColorPoints[i]->mColor.z(),
                    255 * mColorPoints[i]->mColor.w()));
                painter.setPen(QColor(0, 0, 0, 0));
                painter.drawEllipse(visualPosition, innerRadius, innerRadius);
            }
        }
    }
}

void DiffusionCurveRenderer::Controller::SetWindow(Window* newWindow)
{
    mWindow = newWindow;
}

void DiffusionCurveRenderer::Controller::OnWheelMoved(QWheelEvent* event)
{
    if (mWorkMode == WorkMode::Edit)
        mEditModeCamera->OnWheelMoved(event);
    else if (mWorkMode == WorkMode::Vectorization)
        mViewModeCamera->OnWheelMoved(event);
}

void DiffusionCurveRenderer::Controller::OnMousePressed(QMouseEvent* event)
{
    if (mImGuiWantsMouseCapture)
        return;

    mPressedButton = event->button();

    if (event->button() == Qt::LeftButton)
    {
        if (mWorkMode == WorkMode::Edit)
            OnAction((Action)mActionMode, mEditModeCamera->ToOpenGL(event->position()));
    }
    else if (event->button() == Qt::MiddleButton)
    {
        if (mWorkMode == WorkMode::Edit)
            mEditModeCamera->OnMousePressed(event);
        else if (mWorkMode == WorkMode::Vectorization)
            mViewModeCamera->OnMousePressed(event);
    }
}

void DiffusionCurveRenderer::Controller::OnMouseReleased(QMouseEvent* event)
{
    mPressedButton = Qt::NoButton;

    mEditModeCamera->OnMouseReleased(event);
    mViewModeCamera->OnMouseReleased(event);
}

void DiffusionCurveRenderer::Controller::OnMouseMoved(QMouseEvent* event)
{
    if (mImGuiWantsMouseCapture)
        return;

    if (mPressedButton == Qt::LeftButton)
    {
        if (mWorkMode == WorkMode::Edit && mActionMode == ActionMode::Select)
        {
            if (mSelectedColorPoint)
                OnAction(Action::UpdateColorPointPosition, mEditModeCamera->ToOpenGL(event->position()));

            if (mSelectedControlPoint)
                OnAction(Action::UpdateControlPointPosition, mEditModeCamera->ToOpenGL(event->position()));

            if (mSelectedBlurPoint)
                OnAction(Action::UpdateBlurPointPosition, mEditModeCamera->ToOpenGL(event->position()));
        }
    }

    if (mWorkMode == WorkMode::Edit)
        mEditModeCamera->OnMouseMoved(event);
    else if (mWorkMode == WorkMode::Vectorization)
        mViewModeCamera->OnMouseMoved(event);
}

void DiffusionCurveRenderer::Controller::OnKeyPressed(QKeyEvent* event)
{
    if (mImGuiWantsKeyboardCapture)
        return;

    if (mWorkMode == WorkMode::Edit)
    {
        if (event->key() == Qt::Key_Delete)
        {
            if (mSelectedColorPoint)
                OnAction(Action::RemoveColorPoint);
            else if (mSelectedControlPoint)
                OnAction(Action::RemoveControlPoint);
            else if (mSelectedBlurPoint)
                OnAction(Action::RemoveBlurPoint);
            else if (mSelectedCurve)
                OnAction(Action::RemoveCurve);
        }
        else if (event->key() == Qt::Key_Alt)
        {
            if (mSelectedCurve)
                mActionMode = ActionMode::AddColorPoint;
        }
        else if (event->key() == Qt::Key_Control)
        {
            mActionMode = ActionMode::AddControlPoint;
        }
    }
}

void DiffusionCurveRenderer::Controller::OnKeyReleased(QKeyEvent*)
{
    if (mImGuiWantsKeyboardCapture)
        return;
}

void DiffusionCurveRenderer::Controller::Resize(int w, int h)
{
    mWindow->makeCurrent();
    mWidth = w;
    mHeight = h;
    mEditModeCamera->Resize(w, h);
    mViewModeCamera->Resize(w, h);
    mRendererManager->Resize(w, h);
    mBitmapRenderer->Resize(w, h);
    mWindow->doneCurrent();
}