#include "Controller.h"
#include "Util.h"

Controller::Controller(QObject *parent)
    : QObject(parent)
    , mMode(Mode::Select)
    , mMouseLeftButtonPressed(false)
    , mMouseMiddleButtonPressed(false)
    , mMouseRightButtonPressed(false)
    , mMousePressedOnCurve(false)
    , mZoomStep(1.1f)
{
    mWindow = new Window;
    mCurveContainer = new CurveContainer;
    mRendererManager = new RendererManager;
    mProjectionParameters = new ProjectionParameters;
    mTransformer = new Transformer;
    mFileDialog = new QFileDialog;

    mProjectionParameters->left = 0;
    mProjectionParameters->top = 0;
    mProjectionParameters->zoomRatio = 0;
    mProjectionParameters->pixelRatio = 0;
    mProjectionParameters->right = 0;
    mProjectionParameters->bottom = 0;
    mProjectionParameters->width = 0;
    mProjectionParameters->height = 0;

    mTransformer->setProjectionParameters(mProjectionParameters);

    mWindow->setTransformer(mTransformer);
    mWindow->setProjectionParameters(mProjectionParameters);
    mWindow->setRendererManager(mRendererManager);

    mRendererManager->setCurveContainer(mCurveContainer);
    mRendererManager->setProjectionParameters(mProjectionParameters);

    connect(mCurveContainer, &CurveContainer::selectedCurveChanged, mWindow, &Window::onSelectedCurveChanged);
    connect(mCurveContainer, &CurveContainer::selectedControlPointChanged, mWindow, &Window::onSelectedControlPointChanged);
    connect(mCurveContainer, &CurveContainer::selectedColorPointChanged, mWindow, &Window::onSelectedColorPointChanged);

    connect(mWindow, &Window::wheelMoved, this, &Controller::onWheelMoved);
    connect(mWindow, &Window::mousePressed, this, &Controller::onMousePressed);
    connect(mWindow, &Window::mouseReleased, this, &Controller::onMouseReleased);
    connect(mWindow, &Window::mouseMoved, this, &Controller::onMouseMoved);
    connect(mWindow, &Window::action, this, &Controller::onAction);

    connect(mWindow, &Window::keyPressed, this, [=](Qt::Key key) {
        switch (key) {
        case Qt::Key_Control: {
            mPressedKey = Qt::Key_Control;
            mModeBeforeKeyPress = mMode;
            onAction(Action::UpdateMode, (int) Mode::AppendControlPoint);
            break;
        }

        case Qt::Key_Shift: {
            mPressedKey = Qt::Key_Shift;
            mModeBeforeKeyPress = mMode;
            onAction(Action::UpdateMode, (int) Mode::InsertControlPoint);
            break;
        }

        case Qt::Key_Alt: {
            if (mCurveContainer->selectedCurve() != nullptr && mCurveContainer->selectedCurve()->getSize() >= 2) {
                mPressedKey = Qt::Key_Alt;
                mModeBeforeKeyPress = mMode;
                onAction(Action::UpdateMode, (int) Mode::AddColorPoint);
            }
            break;
        }

        case Qt::Key_Delete: {
            if (mCurveContainer->selectedColorPoint())
                onAction(Action::RemoveColorPoint);
            else if (mCurveContainer->selectedControlPoint())
                onAction(Action::RemoveControlPoint);
            else if (mCurveContainer->selectedCurve())
                onAction(Action::RemoveCurve);

            break;
        }

        default:
            break;
        }
    });

    connect(mWindow, &Window::keyReleased, this, [=](Qt::Key key) {
        if (mPressedKey == key) {
            mPressedKey = Qt::Key::Key_unknown;
            onAction(Action::UpdateMode, (int) mModeBeforeKeyPress);
        }
    });

    connect(mFileDialog, &QFileDialog::fileSelected, this, [=](const QString &path) {
        if (!path.isEmpty())
            switch (mLastFileAction) {
            case Action::ShowLoadFromXmlDialog:
                onAction(Action::LoadFromXml, path);
                break;
            case Action::ShowLoadFromJsonDialog:
                onAction(Action::LoadFromJson, path);
                break;
            case Action::ShowSaveAsJsonDialog:
                onAction(Action::SaveAsJson, path);
                break;
            case Action::ShowSaveAsPngDialog:
                onAction(Action::SaveAsPng, path);
                break;
            default:
                break;
            }
    });
}

void Controller::init()
{
    QVector<Curve *> curves = Util::loadCurveDataFromXml(":/Resources/CurveData/zephyr.xml");
    mCurveContainer->addCurves(curves);
    mWindow->showMaximized();
}

void Controller::onAction(Action action, CustomVariant value)
{
    switch (action) {
    case Action::ShowLoadFromJsonDialog:
        mLastFileAction = Action::ShowLoadFromJsonDialog;
        mFileDialog->setFileMode(QFileDialog::ExistingFile);
        mFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        mFileDialog->setNameFilter("*.json");
        mFileDialog->show();
        break;
    case Action::ShowSaveAsJsonDialog:
        mLastFileAction = Action::ShowSaveAsJsonDialog;
        mFileDialog->setFileMode(QFileDialog::AnyFile);
        mFileDialog->setAcceptMode(QFileDialog::AcceptSave);
        mFileDialog->setDefaultSuffix(".json");
        mFileDialog->setNameFilter("*.json");
        mFileDialog->show();
        break;
    case Action::ShowLoadFromXmlDialog:
        mLastFileAction = Action::ShowLoadFromXmlDialog;
        mFileDialog->setFileMode(QFileDialog::ExistingFile);
        mFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        mFileDialog->setNameFilter("*.xml");
        mFileDialog->show();
        break;
    case Action::ShowSaveAsPngDialog:
        mLastFileAction = Action::ShowSaveAsPngDialog;
        mFileDialog->setFileMode(QFileDialog::AnyFile);
        mFileDialog->setAcceptMode(QFileDialog::AcceptSave);
        mFileDialog->setDefaultSuffix(".png");
        mFileDialog->setNameFilter("*.png");
        mFileDialog->show();
        break;
    case Action::UpdateColorRendererMode:
        mWindow->setColorRendererMode((ColorRendererMode) value.toInt());
        mRendererManager->onColorRendererModeChanged((ColorRendererMode) value.toInt());
        break;
    case Action::UpdateMode:
        mMode = (Mode) value.toInt();
        mWindow->setMode(mMode);
        break;
    case Action::UpdateRenderMode:
        mWindow->setRenderMode((RenderMode) value.toInt());
        mRendererManager->onRenderModeChanged((RenderMode) value.toInt());
        break;
    case Action::ClearCanvas:
        mCurveContainer->setSelectedColorPoint(nullptr);
        mCurveContainer->setSelectedControlPoint(nullptr);
        mCurveContainer->setSelectedCurve(nullptr);
        mCurveContainer->clear();
        break;
    case Action::LoadFromXml: {
        QVector<Curve *> curves = Util::loadCurveDataFromXml(value.toString());
        if (!curves.isEmpty()) {
            mCurveContainer->setSelectedColorPoint(nullptr);
            mCurveContainer->setSelectedControlPoint(nullptr);
            mCurveContainer->setSelectedCurve(nullptr);
            mCurveContainer->clear();
            mCurveContainer->addCurves(curves);
        }
        break;
    }
    case Action::SaveAsPng: {
        ColorPoint *colorPoint = mCurveContainer->selectedColorPoint();
        ControlPoint *controlPoint = mCurveContainer->selectedControlPoint();
        Curve *curve = mCurveContainer->selectedCurve();

        mCurveContainer->setSelectedColorPoint(nullptr);
        mCurveContainer->setSelectedControlPoint(nullptr);
        mCurveContainer->setSelectedCurve(nullptr);

        mWindow->setPath(value.toString());
        mWindow->setSave(true);

        mCurveContainer->setSelectedColorPoint(colorPoint);
        mCurveContainer->setSelectedControlPoint(controlPoint);
        mCurveContainer->setSelectedCurve(curve);
        break;
    }

    case Action::LoadFromJson: {
        QVector<Curve *> curves = Util::loadCurveDataFromJson(value.toString());
        if (!curves.isEmpty()) {
            mCurveContainer->setSelectedColorPoint(nullptr);
            mCurveContainer->setSelectedControlPoint(nullptr);
            mCurveContainer->setSelectedCurve(nullptr);
            mCurveContainer->clear();
            mCurveContainer->addCurves(curves);
        }
        break;
    }
    case Action::SaveAsJson: {
        Util::saveCurveDataToJson(mCurveContainer->getCurves(), value.toString());
        break;
    }
    case Action::UpdateRenderQuality:
        mRendererManager->onRenderQualityChanged((RenderQuality) value.toInt());
        mWindow->setRenderQuality((RenderQuality) value.toInt());
        break;
    case Action::UpdateContourThickness:
        mRendererManager->onContourThicknessChanged(value.toFloat());
        break;
    case Action::UpdateDiffusionWidth:
        mRendererManager->onDiffusionWidthChanged(value.toFloat());
        break;
    case Action::UpdateContourColor:
        mRendererManager->onContourColorChanged(value.toVector4D());
        break;
    case Action::UpdateSmoothIterations:
        mRendererManager->onSmoothIterationsChanged(value.toInt());
        break;
    case Action::AddColorPoint: {
        if (mCurveContainer->selectedCurve() && mCurveContainer->selectedCurve()->getSize() >= 2) {
            QVector2D nearbyPosition = value.toVector2D();
            float parameter = mCurveContainer->selectedCurve()->parameterAt(nearbyPosition);
            QVector3D positionOnCurve = mCurveContainer->selectedCurve()->valueAt(parameter).toVector3D();
            QVector3D tangent = mCurveContainer->selectedCurve()->tangentAt(parameter).toVector3D();
            QVector3D direction = (nearbyPosition.toVector3D() - positionOnCurve).normalized();
            QVector3D cross = QVector3D::crossProduct(tangent, direction);

            ColorPoint::Type type = cross.z() > 0 ? ColorPoint::Left : ColorPoint::Right;

            ColorPoint *colorPoint = new ColorPoint;
            colorPoint->setParent(mCurveContainer->selectedCurve());
            colorPoint->setPosition(parameter);
            colorPoint->setType(type);
            colorPoint->setColor(QVector4D(1, 1, 1, 1));

            if (mCurveContainer->selectedCurve()->addColorPoint(colorPoint)) {
                mCurveContainer->setSelectedColorPoint(colorPoint);
                mCurveContainer->setSelectedControlPoint(nullptr);
            } else {
                colorPoint->deleteLater();
            }
        }
        break;
    }
    case Action::RemoveColorPoint: {
        ColorPoint *selectedColorPoint = mCurveContainer->selectedColorPoint();

        Curve *selectedCurve = mCurveContainer->selectedCurve();
        if (selectedCurve && selectedColorPoint) {
            selectedCurve->removeColorPoint(selectedColorPoint);
            mCurveContainer->setSelectedColorPoint(nullptr);
        }
        break;
    }
    case Action::UpdateColorPointColor: {
        ColorPoint *selectedColorPoint = mCurveContainer->selectedColorPoint();
        if (selectedColorPoint)
            selectedColorPoint->setColor(value.toVector4D());

        break;
    }
    case Action::UpdateColorPointPosition: {
        ColorPoint *selectedColorPoint = mCurveContainer->selectedColorPoint();
        if (selectedColorPoint)
            selectedColorPoint->setPosition(value.toFloat());

        break;
    }
    case Action::Select: {
        if (mCurveContainer->selectedCurve()) {
            QVector2D nearbyPoint = value.toVector2D();
            ControlPoint *controlPoint = mCurveContainer->getClosestControlPointOnSelectedCurve(nearbyPoint, 5);
            ColorPoint *colorPoint = mCurveContainer->getClosestColorPointOnSelectedCurve(nearbyPoint, 5);

            if (controlPoint && colorPoint) {
                float distanceToControlPoint = nearbyPoint.distanceToPoint(controlPoint->position());
                float distanceToColorPoint = nearbyPoint.distanceToPoint(colorPoint->getPosition2D());

                mCurveContainer->setSelectedControlPoint(distanceToColorPoint > distanceToControlPoint ? controlPoint : nullptr);
                mCurveContainer->setSelectedColorPoint(distanceToColorPoint < distanceToControlPoint ? colorPoint : nullptr);
                return;
            } else if (controlPoint) {
                mCurveContainer->setSelectedControlPoint(controlPoint);
                mCurveContainer->setSelectedColorPoint(nullptr);
                return;
            } else if (colorPoint) {
                mCurveContainer->setSelectedControlPoint(nullptr);
                mCurveContainer->setSelectedColorPoint(colorPoint);
                return;
            } else {
                mCurveContainer->setSelectedControlPoint(nullptr);
                mCurveContainer->setSelectedColorPoint(nullptr);
            }
        }

        Curve *selectedCurve = mCurveContainer->selectCurve(value.toVector2D(), 10 * mProjectionParameters->zoomRatio);
        mMousePressedOnCurve = selectedCurve ? true : false;
        mCurveContainer->setSelectedCurve(selectedCurve);

        break;
    }
    case Action::AppendControlPoint:
    case Action::InsertControlPoint: {
        if (mCurveContainer->selectedCurve()) {
            if (mCurveContainer->selectedCurve()->getSize() >= Constants::MAX_CONTROL_POINT_COUNT)
                return;

            ControlPoint *controlPoint = new ControlPoint(value.toVector2D());
            controlPoint->setSelected(true);
            mCurveContainer->selectedCurve()->addControlPoint(controlPoint, action == Action::AppendControlPoint);
            mCurveContainer->setSelectedControlPoint(controlPoint);
            mCurveContainer->setSelectedColorPoint(nullptr);
        } else {
            ControlPoint *controlPoint = new ControlPoint(value.toVector2D());
            controlPoint->setSelected(true);

            Bezier *curve = new Bezier();
            curve->addControlPoint(controlPoint);
            mCurveContainer->addCurve(curve);
            mCurveContainer->setSelectedCurve(curve);
            mCurveContainer->setSelectedControlPoint(controlPoint);
            mCurveContainer->setSelectedColorPoint(nullptr);
        }

        break;
    }
    case Action::Move: {
        if (mCurveContainer->selectedCurve())
            mCurveContainer->selectedCurve()->translate(value.toVector2D());
        break;
    }
    case Action::Pan: {
        QVector2D translation = value.toVector2D();
        mProjectionParameters->left += translation.x();
        mProjectionParameters->right += translation.x();
        mProjectionParameters->top += translation.y();
        mProjectionParameters->bottom += translation.y();
        break;
    }
    case Action::RemoveCurve: {
        if (mCurveContainer->selectedCurve()) {
            mCurveContainer->removeCurve(mCurveContainer->selectedCurve());
            mCurveContainer->setSelectedControlPoint(nullptr);
            mCurveContainer->setSelectedCurve(nullptr);
        }
        break;
    }
    case Action::RemoveControlPoint: {
        ControlPoint *selectedControlPoint = mCurveContainer->selectedControlPoint();
        Curve *selectedCurve = mCurveContainer->selectedCurve();
        if (selectedCurve && selectedControlPoint) {
            selectedCurve->removeControlPoint(selectedControlPoint);
            mCurveContainer->setSelectedControlPoint(nullptr);

            if (selectedCurve->getSize() == 0) {
                mCurveContainer->setSelectedCurve(nullptr);
                mCurveContainer->removeCurve(selectedCurve);
            }
        }

        break;
    }
    case Action::UpdateControlPointPosition: {
        if (mCurveContainer->selectedControlPoint())
            mCurveContainer->selectedControlPoint()->setPosition(value.toVector2D());

        break;
    }
    case Action::UpdateControlPointXPosition: {
        if (mCurveContainer->selectedControlPoint()) {
            QVector2D position = mCurveContainer->selectedControlPoint()->position();
            position.setX(value.toFloat());
            mCurveContainer->selectedControlPoint()->setPosition(position);
        }
        break;
    }
    case Action::UpdateControlPointYPosition: {
        if (mCurveContainer->selectedControlPoint()) {
            QVector2D position = mCurveContainer->selectedControlPoint()->position();
            position.setY(value.toFloat());
            mCurveContainer->selectedControlPoint()->setPosition(position);
        }
        break;
    }
    case Action::UpdateCurveZIndex: {
        if (mCurveContainer->selectedCurve()) {
            mCurveContainer->selectedCurve()->setZ(value.toInt());
            mCurveContainer->sortCurves();
        }
        break;
    }

    case Action::ZoomIn: {
        zoom(mProjectionParameters->zoomRatio / mZoomStep, value);

        break;
    }
    case Action::ZoomOut: {
        zoom(mProjectionParameters->zoomRatio * mZoomStep, value);
        break;
    }
    }
}

void Controller::onWheelMoved(QWheelEvent *event)
{
    int delta = event->angleDelta().y();

    if (delta < 0)
        onAction(Action::ZoomOut, event->position().toPoint());
    if (delta > 0)
        onAction(Action::ZoomIn, event->position().toPoint());
}

void Controller::onMousePressed(QMouseEvent *event)
{
    mMouseLeftButtonPressed = event->button() == Qt::LeftButton;
    mMouseMiddleButtonPressed = event->button() == Qt::MiddleButton;
    mMouseRightButtonPressed = event->button() == Qt::RightButton;
    mModeBeforeMousePress = mMode;

    if (mMouseLeftButtonPressed) {
        switch (mMode) {
        case Mode::Select:
        case Mode::AppendControlPoint:
        case Mode::InsertControlPoint:
        case Mode::AddColorPoint: {
            onAction((Action) mMode, mTransformer->mapFromGuiToOpenGL(mMousePosition));
            break;
        }
        default: {
            break;
        }
        }
    }

    if (mMouseMiddleButtonPressed)
        onAction(Action::UpdateMode, (int) Mode::Pan);

    if (mMouseRightButtonPressed)
        onAction(Action::UpdateMode, (int) Mode::MoveCurve);
}

void Controller::onMouseReleased(QMouseEvent *event)
{
    if (mMouseMiddleButtonPressed)
        onAction(Action::UpdateMode, (int) mModeBeforeMousePress);

    if (mMouseRightButtonPressed)
        onAction(Action::UpdateMode, (int) mModeBeforeMousePress);

    mMouseLeftButtonPressed = false;
    mMouseMiddleButtonPressed = false;
    mMouseRightButtonPressed = false;
}

void Controller::onMouseMoved(QMouseEvent *event)
{
    switch (mMode) {
    case Mode::AddColorPoint:
        break;

    case Mode::Pan: {
        QVector2D translation = QVector2D(mMousePosition - event->pos()) * mProjectionParameters->zoomRatio;
        onAction(Action::Pan, translation);
        break;
    }

    case Mode::Select: {
        QVector2D position = mTransformer->mapFromGuiToOpenGL(event->pos());
        if (mMouseLeftButtonPressed) {
            if (mCurveContainer->selectedControlPoint()) {
                onAction(Action::UpdateControlPointPosition, position);
            } else if (mCurveContainer->selectedColorPoint()) {
                if (mCurveContainer->selectedCurve()) {
                    onAction(Action::UpdateColorPointPosition, mCurveContainer->selectedCurve()->parameterAt(position, 10000));
                }
            }
        }
        break;
    }

    case Mode::MoveCurve: {
        if (mMouseRightButtonPressed || mMouseLeftButtonPressed) {
            if (mCurveContainer->selectedCurve()) {
                QVector2D translation = QVector2D(event->pos() - mMousePosition) * mProjectionParameters->zoomRatio;
                onAction(Action::Move, translation);

            } else {
                QVector2D position = mTransformer->mapFromGuiToOpenGL(mMousePosition);
                onAction(Action::Select, position);
            }
        }

        break;
    }
    default:
        break;
    }

    mMousePosition = event->pos();
}

bool Controller::cursorInsideBoundingBox(QPointF position, QMarginsF margins)
{
    if (mCurveContainer->selectedCurve()) {
        QRectF boundingBox = mCurveContainer->selectedCurve()->getBoundingBox();
        boundingBox = boundingBox.marginsAdded(margins);
        return boundingBox.contains(mTransformer->mapFromGuiToOpenGL(position).toPointF());
    } else
        return false;
}

void Controller::zoom(float newZoomRatio, CustomVariant cursorPositionVariant)
{
    if (qFuzzyCompare(newZoomRatio, mProjectionParameters->zoomRatio))
        return;

    QPoint cursorPosition;
    float width = mWindow->width();
    float height = mWindow->height();

    if (cursorPositionVariant.isValid())
        cursorPosition = cursorPositionVariant.toPoint();
    else
        cursorPosition = QPoint(width / 2, height / 2);

    QVector2D positionBeforeZoom = mTransformer->mapFromGuiToOpenGL(cursorPosition);

    if (newZoomRatio >= 2)
        newZoomRatio = 2;
    if (newZoomRatio <= 1 / 16.0)
        newZoomRatio = 1 / 16.0f;

    mProjectionParameters->zoomRatio = newZoomRatio;

    // Zoom
    mProjectionParameters->right = mProjectionParameters->left + width * mProjectionParameters->zoomRatio;
    mProjectionParameters->bottom = mProjectionParameters->top + height * mProjectionParameters->zoomRatio;

    QVector2D positionAfterZoom = mTransformer->mapFromGuiToOpenGL(cursorPosition);

    float dx = positionBeforeZoom.x() - positionAfterZoom.x();
    float dy = positionBeforeZoom.y() - positionAfterZoom.y();

    mProjectionParameters->left += dx;
    mProjectionParameters->right += dx;
    mProjectionParameters->bottom += dy;
    mProjectionParameters->top += dy;
}
