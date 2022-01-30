#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "CurveContainer.h"
#include "CustomVariant.h"
#include "Transformer.h"
#include "Types.h"
#include "Window.h"

#include <Curves/Curve.h>
#include <Renderers/RendererManager.h>

#include <QFileDialog>
#include <QObject>
#include <QVariant>

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);

public slots:
    void onAction(Action action, CustomVariant value = CustomVariant());
    void init();

private slots:
    void onWheelMoved(QWheelEvent *event);
    void onMousePressed(QMouseEvent *event);
    void onMouseReleased(QMouseEvent *event);
    void onMouseMoved(QMouseEvent *event);

    bool cursorInsideBoundingBox(QPointF position, QMarginsF margins = QMarginsF(-20, -20, 20, 20));
    void zoom(float newZoomRatio, CustomVariant cursorPositionVariant = CustomVariant());

private:
    Window *mWindow;
    CurveContainer *mCurveContainer;
    Transformer *mTransformer;
    RendererManager *mRendererManager;

    ProjectionParameters *mProjectionParameters;

    Mode mModeBeforeKeyPress;
    Mode mModeBeforeMousePress;
    Mode mMode;

    bool mMouseLeftButtonPressed;
    bool mMouseMiddleButtonPressed;
    bool mMouseRightButtonPressed;
    bool mMousePressedOnCurve;
    QPointF mMousePosition;

    float mZoomStep;

    Qt::Key mPressedKey;

    QFileDialog *mFileDialog;
    Action mLastFileAction;
};

#endif // CONTROLLER_H
