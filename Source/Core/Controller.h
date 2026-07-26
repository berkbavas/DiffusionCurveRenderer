#pragma once

#include "Core/Constants.h"
#include "Curve/Curve.h"
#include "Structs/Enums.h"
#include "Util/Macros.h"

#include <opencv2/core/mat.hpp>

#include <QMouseEvent>
#include <QOpenGLExtraFunctions>
#include <QThread>
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class Window;
    class RendererManager;
    class OrthographicCamera;
    class CurveContainer;
    class EventHandler;
    class OverlayPainter;
    class ImGuiWindow;
    class BitmapRenderer;
    class VectorizationManager;

    class Controller : public QObject, protected QOpenGLExtraFunctions
    {
        Q_OBJECT
      public:
        explicit Controller(QObject* pParent = nullptr);
        ~Controller();

        void Run();

      public slots:
        // Core Events
        void Initialize();
        void Resize(int Width, int Height);
        void Render(float Ifps);

        // Input Events
        void OnKeyPressed(QKeyEvent* pEvent);
        void OnKeyReleased(QKeyEvent* pEvent);
        void OnMousePressed(QMouseEvent* pEvent);
        void OnMouseReleased(QMouseEvent* pEvent);
        void OnMouseMoved(QMouseEvent* pEvent);
        void OnWheelMoved(QWheelEvent* pEvent);

        void OnVectorizationViewOptionChanged(VectorizationViewOption Option);
        void OnGaussianStackLayerChanged(int Layer);
        void OnEdgeStackLayerChanged(int Layer);

        void OnImageLoaded(cv::Mat Image);
        void OnVectorizationStageFinished(VectorizationStage Stage, QVariant AdditionalData);
        void OnVectorizationFinished(const QVector<CurvePtr>& Curves);

      private:
        void OnSelectedCurveChanged(CurvePtr SelectedCurve);
        void SetWorkMode(WorkMode WorkMode);
        void ClearCanvas();
        void DuplicateCurve(CurvePtr Curve);
        void OnBackgroundColorChanged(const QVector4D& Color);
        void OnShowGridChanged(bool Show);
        void OnResetView();
        void OnZoomToFit();

        float mDevicePixelRatio{ 1.0f };
        float mWidth{ INITIAL_WIDTH };
        float mHeight{ INITIAL_HEIGHT };

        RendererManager* mRendererManager;
        OrthographicCamera* mCamera;
        CurveContainer* mCurveContainer;
        EventHandler* mEventHandler;
        OverlayPainter* mOverlayPainter;
        ImGuiWindow* mImGuiWindow;
        BitmapRenderer* mBitmapRenderer;
        VectorizationManager* mVectorizationManager;

        Window* mWindow;

        CurvePtr mSelectedCurve{ nullptr };

        RenderModes mRenderModes{ RenderMode::Contour | RenderMode::Diffusion };
        WorkMode mWorkMode{ WorkMode::CurveEditing };

        QThread* mVectorizationManagerThread;

        // New state for enhancements
        QVector4D mBackgroundColor{ 0.2f, 0.2f, 0.2f, 1.0f };
        bool mShowGrid{ false };
        float mGridSpacing{ 50.0f };
    };
}
