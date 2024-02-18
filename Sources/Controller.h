#pragma once

#include "Common.h"
#include "CurveManager.h"
#include "CustomVariant.h"
#include "EditModeCamera.h"
#include "BitmapRenderer.h"
#include "RendererManager.h"
#include "ShaderManager.h"
#include "Vectorizer.h"
#include "ViewModeCamera.h"

#include <QObject>

#include <imgui.h>
#include <QFileDialog>
#include <QFuture>
#include <QPen>
#include <QtImGui.h>

namespace DiffusionCurveRenderer
{
    class Window;

    class Controller : public QObject, protected QOpenGLExtraFunctions
    {
        Q_OBJECT
    public:
        explicit Controller(QObject* parent = nullptr);
        virtual ~Controller();

        void OnAction(Action action, CustomVariant value = CustomVariant());

        void Init();
        void OnWheelMoved(QWheelEvent* event);
        void OnMousePressed(QMouseEvent* event);
        void OnMouseReleased(QMouseEvent* event);
        void OnMouseMoved(QMouseEvent* event);
        void OnKeyPressed(QKeyEvent* event);
        void OnKeyReleased(QKeyEvent* event);
        void Resize(int w, int h);
        void Render(float ifps);

        void SetWindow(Window* newWindow);

    signals:
        void Load(QString path);
        void Draw();

    private:
        void DrawGUI();
        void DrawPainter();

        enum class FileDialogType { //
            LoadFromXML,
            LoadFromJSON,
            SaveAsPNG,
            SaveAsJSON,
            LoadVectorizerImage,
        };

    private:
        QThread mVectorizerThread;

        RendererManager* mRendererManager;
        CurveManager* mCurveManager;
        ShaderManager* mShaderManager;
        BitmapRenderer* mBitmapRenderer;
        Vectorizer* mVectorizer;

        QVector<Manager*> mManagers;

        Window* mWindow;
        EditModeCamera* mEditModeCamera;
        ViewModeCamera* mViewModeCamera;

        float mIfps;
        bool mImGuiWantsMouseCapture;
        bool mImGuiWantsKeyboardCapture;

        WorkMode mWorkMode;
        SubWorkMode mSubWorkMode;
        SubWorkMode mPreviousSubWorkMode;
        ActionMode mActionMode;
        RenderMode mRenderMode;
        float mWidth;
        float mHeight;
        float mPixelRatio;

        float mGlobalContourThickness;
        float mGlobalDiffusionWidth;
        float mGlobalDiffusionGap;
        QVector4D mGlobalContourColor;
        float mGlobalBlurStrength;

        int mSmoothIterations;
        int mQualityFactor;

        float mVoidThreshold;

        // GUI
        QPen mDashedPen;
        QPen mSolidPen;
        QPen mDenseDashedPen;
        QPointF mPreviousMousePosition;
        Qt::MouseButton mPressedButton;
        bool mVectorizerImageLoaded;
        bool mShowFileDailog;
        FileDialogType mFileDailogType;

        // Aux
        Bezier* mSelectedCurve;
        ControlPoint* mSelectedControlPoint;
        ColorPoint* mSelectedColorPoint;
        BlurPoint* mSelectedBlurPoint;

        QList<ControlPoint*> mControlPoints;
        QList<ColorPoint*> mColorPoints;
        QList<BlurPoint*> mBlurPoints;
    };
}
