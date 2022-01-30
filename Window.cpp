#include "Window.h"

#include "Renderers/RendererManager.h"
#include "Util.h"

#include <imgui.h>
#include <QtImGui.h>

#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QVector4D>

Window::Window(QWindow *parent)
    : QOpenGLWindow(QOpenGLWindow::UpdateBehavior::NoPartialUpdate, parent)
    , mRendererManager(nullptr)
    , mSelectedCurve(nullptr)
    , mSelectedControlPoint(nullptr)
    , mSelectedColorPoint(nullptr)
    , mMode(Mode::Select)
    , mRenderMode(RenderMode::Diffuse + RenderMode::Contours)
    , mColorRendererMode(ColorRendererMode::TriangleStrip)
    , mRenderQuality(RenderQuality::Low)
    , mInit(false)
    , mImguiWantCapture(false)
    , mMouseLeftButtonPressed(false)
    , mMouseRightButtonPressed(false)
    , mMouseMiddleButtonPressed(false)
    , mMousePosition(0, 0)
    , mDiffusionWidth(2.0f)
    , mContourThickness(1.0f)
    , mSmoothIterations(40)
    , mSave(false)
{
    for (int i = 0; i < 4; ++i)
        mHandles[i].setSize(QSize(8, 8));

    mDashedPen.setDashPattern({8, 8});
    mDashedPen.setWidthF(0.5f);
    mDashedPen.setJoinStyle(Qt::MiterJoin);

    mSolidPen.setWidthF(0.5f);
    mSolidPen.setJoinStyle(Qt::MiterJoin);

    connect(this, &QOpenGLWindow::frameSwapped, this, [=]() { update(); });
}

void Window::initializeGL()
{
    initializeOpenGLFunctions();
    QtImGui::initialize(this);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);

    mProjectionParameters->left = 0;
    mProjectionParameters->top = 0;
    mProjectionParameters->zoomRatio = 1.0f;
    mProjectionParameters->pixelRatio = devicePixelRatioF();
    mProjectionParameters->right = mProjectionParameters->left + width() * mProjectionParameters->zoomRatio;
    mProjectionParameters->bottom = mProjectionParameters->top + height() * mProjectionParameters->zoomRatio;
    mProjectionParameters->width = width() * mProjectionParameters->pixelRatio;
    mProjectionParameters->height = height() * mProjectionParameters->pixelRatio; // Flip y-axis

    mRendererManager->init();
}

void Window::paintGL()
{
    mProjectionParameters->pixelRatio = devicePixelRatioF();
    mProjectionParameters->right = mProjectionParameters->left + width() * mProjectionParameters->zoomRatio;
    mProjectionParameters->bottom = mProjectionParameters->top + height() * mProjectionParameters->zoomRatio;
    mProjectionParameters->width = width() * mProjectionParameters->pixelRatio;
    mProjectionParameters->height = height() * mProjectionParameters->pixelRatio; // Flip y-axis

    mRendererManager->render();

    if (mSave) {
        mSave = false;
        grabFramebuffer().save(mPath);
    }

    if (mSelectedCurve) {
        QRectF boundingBox = mTransformer->mapFromOpenGLToGui(mSelectedCurve->getBoundingBox());
        mHandles[0].moveCenter(boundingBox.topLeft());
        mHandles[1].moveCenter(boundingBox.bottomLeft());
        mHandles[2].moveCenter(boundingBox.topRight());
        mHandles[3].moveCenter(boundingBox.bottomRight());
    }

    updatePainter();
    updateCursor();

    //////////////////////////////////////////

    {
        mImguiWantCapture = ImGui::GetIO().WantCaptureMouse;

        if (mImguiWantCapture)
            ImGui::GetIO().ConfigFlags = 0;
        else
            ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

        QtImGui::newFrame();
        ImGui::SetNextWindowSize(ImVec2(420, 820), ImGuiCond_FirstUseEver);

        ImGui::Begin("Controls", NULL, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Load from XML"))
                    emit action(Action::ShowLoadFromXmlDialog);

                if (ImGui::MenuItem("Load from JSON"))
                    emit action(Action::ShowLoadFromJsonDialog);

                ImGui::Separator();

                if (ImGui::MenuItem("Save as PNG"))
                    emit action(Action::ShowSaveAsPngDialog);

                if (ImGui::MenuItem("Save as JSON"))
                    emit action(Action::ShowSaveAsJsonDialog);

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Action Modes
        if (!ImGui::CollapsingHeader("Action Modes")) {
            ImGui::Text("Actions Modes");

            int mode = (int) mMode;

            ImGui::RadioButton("Select (Mouse Left Click)", &mode, 0);
            ImGui::RadioButton("Append Control Point (Ctrl)", &mode, 1);
            ImGui::RadioButton("Insert Control Point (Shift)", &mode, 2);
            ImGui::RadioButton("Add Color Point (Alt)", &mode, 3);
            ImGui::RadioButton("Move Curve (Mouse Left Button)", &mode, 4);
            ImGui::RadioButton("Pan (Mouse Middle Button)", &mode, 5);

            if (mode != (int) mMode)
                emit action(Action::UpdateMode, mode);
        }

        // Curve
        {
            if (!ImGui::CollapsingHeader("Curve")) {
                int z = mSelectedCurve ? mSelectedCurve->z() : 0;

                ImGui::BeginDisabled(!mSelectedCurve);

                ImGui::Text("Curve");

                if (ImGui::InputInt("z", &z))
                    emit action(Action::UpdateCurveZIndex, z);

                if (ImGui::Button("Remove Curve"))
                    emit action(Action::RemoveCurve);

                ImGui::EndDisabled();
            }
        }

        // Control Point
        {
            if (!ImGui::CollapsingHeader("Control Point")) {
                float x = mSelectedControlPoint ? mSelectedControlPoint->position().x() : 0;
                float y = mSelectedControlPoint ? mSelectedControlPoint->position().y() : 0;
                int index = mSelectedControlPoint ? mSelectedControlPoint->index() : 0;

                ImGui::BeginDisabled(!mSelectedControlPoint);

                ImGui::Text("Control Point");
                ImGui::Text("Index: %d", index);

                if (ImGui::InputFloat("x", &x, 0.1f, 1.0f, "%.3f"))
                    emit action(Action::UpdateControlPointXPosition, x);

                if (ImGui::InputFloat("y", &y, 0.1f, 1.0f, "%.3f"))
                    emit action(Action::UpdateControlPointYPosition, y);

                if (ImGui::Button("Remove Control Point")) {
                    emit action(Action::RemoveControlPoint);
                }

                ImGui::EndDisabled();
            }
        }

        // Color Point
        {
            if (!ImGui::CollapsingHeader("Color Point")) {
                float t = mSelectedColorPoint ? mSelectedColorPoint->position() : 0;
                const char *type = mSelectedColorPoint ? (mSelectedColorPoint->type() == ColorPoint::Left ? "Left" : "Right") : "";
                float color[4] = {0, 0, 0, 0};

                if (mSelectedColorPoint) {
                    color[0] = mSelectedColorPoint->color().x();
                    color[1] = mSelectedColorPoint->color().y();
                    color[2] = mSelectedColorPoint->color().z();
                    color[3] = mSelectedColorPoint->color().w();
                }

                ImGui::BeginDisabled(!mSelectedColorPoint);

                ImGui::Text("Type: %s", type);

                if (ImGui::SliderFloat("Position", &t, 0.0f, 1.0f, "%.3f"))
                    emit action(Action::UpdateColorPointPosition, t);

                ImGui::Text("Color:");

                if (ImGui::ColorEdit4("", (float *) &color))
                    emit action(Action::UpdateColorPointColor, QVector4D(color[0], color[1], color[2], color[3]));

                if (ImGui::Button("Remove Color Point"))
                    emit action(Action::RemoveColorPoint);

                ImGui::EndDisabled();
            }
        }

        // Render Settings
        {
            if (!ImGui::CollapsingHeader("Render Settings")) {
                RenderMode renderMode = mRenderMode;

                bool contours = renderMode & RenderMode::Contours;
                bool diffuse = renderMode & RenderMode::Diffuse;

                ImGui::Text("Render Mode");
                if (ImGui::Checkbox("Render Contours", &contours))
                    renderMode = renderMode + RenderMode::Contours;

                ImGui::SameLine();

                if (ImGui::Checkbox("Render Diffusion", &diffuse))
                    renderMode = renderMode + RenderMode::Diffuse;

                if (renderMode != mRenderMode)
                    emit action(Action::UpdateRenderMode, (int) renderMode);

                int colorRendererMode = (int) mColorRendererMode;
                ImGui::Text("Primitive");
                ImGui::RadioButton("Line Strip", &colorRendererMode, 0);
                ImGui::SameLine();
                ImGui::RadioButton("Triangle Strip", &colorRendererMode, 1);
                ImGui::Spacing();

                if (colorRendererMode != (int) mColorRendererMode)
                    emit action(Action::UpdateColorRendererMode, colorRendererMode);

                int renderQuality = (int) mRenderQuality;

                const char *names[3] = {"Low", "Fair", "High"};
                const char *elem_name = (renderQuality >= 0 && renderQuality < 3) ? names[renderQuality] : "Unknown";

                if (ImGui::SliderInt("Render Quality", &renderQuality, 0, 2, elem_name))
                    emit action(Action::UpdateRenderQuality, renderQuality);

                if (ImGui::SliderInt("Smooth Iterations", &mSmoothIterations, 1, 50))
                    emit action(Action::UpdateSmoothIterations, mSmoothIterations);

                if (ImGui::SliderFloat("Diffusion Width", &mDiffusionWidth, 1.0f, 10.0f, "%.3f"))
                    emit action(Action::UpdateDiffusionWidth, mDiffusionWidth);

                if (ImGui::SliderFloat("Contour Thickness", &mContourThickness, 1.0f, 10.0f, "%.3f"))
                    emit action(Action::UpdateContourThickness, mContourThickness);

                if (ImGui::Button("Clear Canvas"))
                    emit action(Action::ClearCanvas);

                ImGui::Spacing();

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }
        }

        ImGui::End();

        glViewport(0, 0, width(), height());
        ImGui::Render();
        QtImGui::render();

        //////////////////////////////////////////
    }
}

void Window::updatePainter()
{
    if (mRenderMode & RenderMode::Diffuse) {
        mDashedPen.setColor(QColor(255, 255, 255));
        mSolidPen.setColor(QColor(255, 255, 255));
    } else {
        mDashedPen.setColor(QColor(0, 0, 0));
        mSolidPen.setColor(QColor(0, 0, 0));
    }

    QOpenGLPaintDevice device(width() * QPaintDevice::devicePixelRatioF(), height() * QPaintDevice::devicePixelRatioF());
    QPainter painter(&device);
    painter.setRenderHint(QPainter::Antialiasing, true);

    switch (mMode) {
    case Mode::Pan:
        break;
    case Mode::AddColorPoint:
    case Mode::AppendControlPoint:
    case Mode::InsertControlPoint:
    case Mode::Select: {
        if (mSelectedCurve) {
            QVector<const ControlPoint *> controlPoints = mSelectedCurve->getControlPoints();

            // Control polygon
            painter.setPen(mDashedPen);
            painter.setBrush(QBrush());
            for (int i = 0; i < controlPoints.size() - 1; ++i) {
                QPointF p0 = mTransformer->mapFromOpenGLToGui(controlPoints[i]->position());
                QPointF p1 = mTransformer->mapFromOpenGLToGui(controlPoints[i + 1]->position());
                painter.drawLine(p0, p1);
            }

            // Control Points
            for (int j = 0; j < controlPoints.size(); ++j) {
                QPointF center = mTransformer->mapFromOpenGLToGui(controlPoints[j]->position());
                painter.setPen(QColor(0, 0, 0, 0));

                // Outer disk
                float outerRadius = controlPoints[j]->selected() ? 16 : 12;
                outerRadius = qMin(outerRadius, outerRadius / mProjectionParameters->zoomRatio);
                painter.setBrush(QColor(128, 128, 128, 128));
                painter.drawEllipse(center, outerRadius, outerRadius);

                // Inner disk
                float innerRadius = 6;
                innerRadius = qMin(innerRadius, innerRadius / mProjectionParameters->zoomRatio);
                painter.setBrush(QColor(255, 255, 255));
                painter.drawEllipse(center, innerRadius, innerRadius);
            }

            // Color points
            QVector<const ColorPoint *> colorPoints = mSelectedCurve->getRightColorPoints();
            colorPoints << mSelectedCurve->getLeftColorPoints();

            for (int i = 0; i < colorPoints.size(); ++i) {
                QPointF center = mTransformer->mapFromOpenGLToGui(colorPoints[i]->getPosition2D());
                painter.setPen(QColor(0, 0, 0, 0));

                // Outer disk
                float outerRadius = colorPoints[i]->selected() ? 16 : 12;
                outerRadius = qMin(outerRadius, outerRadius / mProjectionParameters->zoomRatio);
                painter.setBrush(QColor(0, 0, 0, 128));
                painter.drawEllipse(center, outerRadius, outerRadius);

                // Inner disk
                float innerRadius = 6;
                innerRadius = qMin(innerRadius, innerRadius / mProjectionParameters->zoomRatio);
                painter.setBrush(QColor(255 * colorPoints[i]->color().x(),
                                        255 * colorPoints[i]->color().y(),
                                        255 * colorPoints[i]->color().z(),
                                        255 * colorPoints[i]->color().w()));
                painter.drawEllipse(center, innerRadius, innerRadius);
            }
        }
        break;
    }
    case Mode::MoveCurve: {
        if (mSelectedCurve) {
            // Draw bounding box
            QRectF boundingBox = mTransformer->mapFromOpenGLToGui(mSelectedCurve->getBoundingBox());
            painter.setPen(mDashedPen);
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.drawRect(boundingBox);
            painter.setRenderHint(QPainter::Antialiasing, true);

            painter.setPen(mSolidPen);

            // Draw pivot point
            QPointF center = boundingBox.center();
            painter.drawEllipse(center, 6, 6);
            painter.drawLine(center.x() - 10, center.y() + 0.5, center.x() + 10, center.y() + 0.5);
            painter.drawLine(center.x() + 0.5, center.y() - 10, center.x() + 0.5, center.y() + 10);

            // Draw corners
            painter.setRenderHint(QPainter::Antialiasing, false);
            for (int i = 0; i < 4; ++i) {
                painter.drawRect(mHandles[i]);
            }
            painter.setRenderHint(QPainter::Antialiasing, true);
        }
        break;
    }
    }
}

void Window::updateCursor()
{
    if (!mImguiWantCapture)
        switch (mMode) {
        case Mode::AppendControlPoint:
            setCursor(Qt::CrossCursor);
            break;
        case Mode::InsertControlPoint:
            setCursor(Qt::CrossCursor);
            break;
        case Mode::AddColorPoint:
            setCursor(Qt::CrossCursor);
            break;
        case Mode::Pan:
            setCursor(mMouseLeftButtonPressed ? Qt::ClosedHandCursor : Qt::OpenHandCursor);
            break;
        case Mode::Select:
            setCursor(Qt::ArrowCursor);
            break;
        case Mode::MoveCurve: {
            if (mSelectedCurve)
                setCursor(Qt::SizeAllCursor);
            else
                setCursor(Qt::ArrowCursor);
            break;
        }
        }
}

void Window::wheelEvent(QWheelEvent *event)
{
    if (mImguiWantCapture)
        return;

    emit wheelMoved(event);
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if (mImguiWantCapture)
        return;

    mMouseRightButtonPressed = event->button() == Qt::RightButton;
    mMouseMiddleButtonPressed = event->button() == Qt::MiddleButton;
    mMouseLeftButtonPressed = event->button() == Qt::LeftButton;

    emit mousePressed(event);
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if (mImguiWantCapture)
        return;

    mMouseRightButtonPressed = false;
    mMouseMiddleButtonPressed = false;
    mMouseLeftButtonPressed = false;

    emit mouseReleased(event);
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if (mImguiWantCapture)
        return;

    emit mouseMoved(event);
    mMousePosition = event->pos();
}

void Window::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed((Qt::Key) event->key());
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleased((Qt::Key) event->key());
}

void Window::setPath(const QString &newPath)
{
    mPath = newPath;
}

void Window::setSave(bool newSave)
{
    mSave = newSave;
}

void Window::setRenderQuality(RenderQuality newRenderQuality)
{
    mRenderQuality = newRenderQuality;
}

void Window::setColorRendererMode(ColorRendererMode newColorRendererMode)
{
    mColorRendererMode = newColorRendererMode;
}

void Window::setRenderMode(RenderMode newRenderMode)
{
    mRenderMode = newRenderMode;
}

void Window::setMode(Mode newMode)
{
    mMode = newMode;
}

void Window::setRendererManager(RendererManager *newRendererManager)
{
    mRendererManager = newRendererManager;
}

void Window::setProjectionParameters(ProjectionParameters *newProjectionParameters)
{
    mProjectionParameters = newProjectionParameters;
}

void Window::setTransformer(Transformer *newTransformer)
{
    mTransformer = newTransformer;
}

void Window::onSelectedControlPointChanged(const ControlPoint *selectedControlPoint)
{
    mSelectedControlPoint = selectedControlPoint;
}

void Window::onSelectedCurveChanged(const Curve *selectedCurve)
{
    mSelectedCurve = selectedCurve;
}

void Window::onSelectedColorPointChanged(const ColorPoint *selectedColorPoint)
{
    mSelectedColorPoint = selectedColorPoint;
}
