#ifndef WINDOW_H
#define WINDOW_H

#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWindow>
#include <QPen>
#include <QWheelEvent>

#include "Curves/Bezier.h"
#include "CustomVariant.h"
#include "Transformer.h"
#include "Types.h"

class RendererManager;

class Window : public QOpenGLWindow, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    Window(QWindow *parent = nullptr);

    void setTransformer(Transformer *newTransformer);
    void setProjectionParameters(ProjectionParameters *newProjectionParameters);
    void setRendererManager(RendererManager *newRendererManager);
    void setMode(Mode newMode);
    void setRenderMode(RenderMode newRenderMode);
    void setColorRendererMode(ColorRendererMode newColorRendererMode);
    void setRenderQuality(RenderQuality newRenderQuality);

    void setSave(bool newSave);

    void setPath(const QString &newPath);

signals:
    void action(Action action, CustomVariant variant = CustomVariant());
    void wheelMoved(QWheelEvent *event);
    void mousePressed(QMouseEvent *event);
    void mouseReleased(QMouseEvent *event);
    void mouseMoved(QMouseEvent *event);
    void keyPressed(Qt::Key key);
    void keyReleased(Qt::Key key);

public slots:
    void onSelectedControlPointChanged(const ControlPoint *selectedControlPoint);
    void onSelectedCurveChanged(const Curve *selectedCurve);
    void onSelectedColorPointChanged(const ColorPoint *selectedColorPoint);

private slots:
    void updatePainter();
    void updateCursor();

protected:
    void initializeGL() override;
    void paintGL() override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

private:
    Transformer *mTransformer;
    RendererManager *mRendererManager;

    const Curve *mSelectedCurve;
    const ControlPoint *mSelectedControlPoint;
    const ColorPoint *mSelectedColorPoint;
    ProjectionParameters *mProjectionParameters;

    Mode mMode;
    RenderMode mRenderMode;
    ColorRendererMode mColorRendererMode;
    RenderQuality mRenderQuality;

    bool mInit;
    bool mDirty;

    bool mImguiWantCapture;
    bool mMouseLeftButtonPressed;
    bool mMouseRightButtonPressed;
    bool mMouseMiddleButtonPressed;
    QPointF mMousePosition;
    QRectF mHandles[4];

    QPen mDashedPen;
    QPen mSolidPen;

    float mDiffusionWidth;
    float mContourThickness;
    int mSmoothIterations;

    bool mSave;
    QString mPath;
};

#endif // WINDOW_H
