#pragma once

#include "Core/Constants.h"
#include "Curve/Curve.h"
#include "Structs/Enums.h"
#include "Util/Macros.h"

#include <QMouseEvent>
#include <QOpenGLExtraFunctions>
#include <QThread>
#include <opencv2/core/mat.hpp>

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
        DISABLE_COPY(Controller);

        Q_OBJECT
      public:
        explicit Controller(QObject* parent = nullptr);
        ~Controller();

        void Run();

      public slots:
        // Core Events
        void Initialize();
        void Resize(int w, int h);
        void Render(float ifps);

        // Input Events
        void OnKeyPressed(QKeyEvent*);
        void OnKeyReleased(QKeyEvent*);
        void OnMousePressed(QMouseEvent*);
        void OnMouseReleased(QMouseEvent*);
        void OnMouseMoved(QMouseEvent*);
        void OnWheelMoved(QWheelEvent*);

        void OnVectorizationOptionChanged(VectorizationOption option);
        void OnImageLoaded(cv::Mat image);
        void OnGaussianStackLayerChanged(int layer);
        void OnGaussianStackFinished(int maximumLayer);
        void OnEdgeStackLayerChanged(int layer);
        void OnEdgeStackFinished(int maximumLayer);

      private:
        void OnSelectedCurveChanged(CurvePtr selectedCurve);
        void SetWorkMode(WorkMode workMode);

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
    };
}
