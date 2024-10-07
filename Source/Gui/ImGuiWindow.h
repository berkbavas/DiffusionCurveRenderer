#pragma once

#include "Curve/Spline.h"
#include "Structs/Enums.h"
#include "Util/Macros.h"

#include <QObject>
#include <QVariant>

namespace DiffusionCurveRenderer
{
    class CurveContainer;
    class RendererManager;

    class ImGuiWindow : public QObject
    {
        Q_OBJECT
      public:
        explicit ImGuiWindow(QObject* parent);

        void Draw();

        void SetSelectedCurve(CurvePtr selectedCurve);
        void SetSelectedControlPoint(ControlPointPtr point);
        void SetSelectedColorPoint(ColorPointPtr point);
        void SetRenderMode(RenderMode mode, bool on);
        void SetWorkMode(WorkMode workMode);
        void SetVectorizationState(VectorizationState state);
        void SetVectorizationOption(VectorizationOption option);
        void SetGaussianStackLayer(int layer);

        int GetGaussianStackLayer() const { return mGaussianStackLayer; }
        int GetEdgeStackLayer() const { return mEdgeStackLayer; }

      signals:
        void SelectedCurveChanged(CurvePtr selectedCurve);
        void SelectedControlPointChanged(ControlPointPtr point);
        void SelectedColorPointChanged(ColorPointPtr point);

        void RenderModesChanged(RenderModes modes);
        void WorkModeChanged(WorkMode workMode);
        void VectorizationOptionChanged(VectorizationOption option);
        void GaussianStackLayerChanged(int layer);
        void EdgeStackLayerChanged(int layer);

        void LoadImage(const QString& path);
        void Vectorize(int edgeLevel);

      private:
        void DrawWorkModes();

        void DrawVectorizationOptions();
        void DrawVectorizationProgressBar();

        void DrawCurveEditingSettings();
        void DrawMenuBar();
        void DrawHintTexts();
        void DrawRenderMode();
        void DrawCurveHeader();
        void DrawRenderSettings();
        void DrawStats();

        float mGlobalContourThickness;
        float mGlobalDiffusionWidth;
        float mGlobalDiffusionGap;
        float mGlobalBlurStrength;

        int mSmoothIterations;
        int mFrambufferSize;

        int mFrambufferSizeIndex;

        WorkMode mWorkMode{ WorkMode::CurveEditing };
        VectorizationState mVectorizationState{ VectorizationState::Ready };
        VectorizationOption mVectorizationOption{ VectorizationOption::ViewOriginalImage };
        int mGaussianStackLayer{ 0 };
        int mEdgeStackLayer{ 0 };

        RenderModes mRenderModes{ RenderMode::Diffusion | RenderMode::Contour };

        bool mRenderModeDiffusion{ true };
        bool mRenderModeContour{ true };

        CurvePtr mSelectedCurve{ nullptr };
        ControlPointPtr mSelectedControlPoint{ nullptr };
        ColorPointPtr mSelectedColorPoint{ nullptr };

        DEFINE_MEMBER(float, VectorizationProgress, 0.0f); // [0,1]
        DEFINE_MEMBER(int, MaximumGaussianStackLayer, 10);
        DEFINE_MEMBER(int, MaximumEdgeStackLayer, 10);

        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
        DEFINE_MEMBER_PTR(RendererManager, RendererManager);
        DEFINE_MEMBER(bool, ImageLoaded, false);

        static constexpr const char* FRAME_BUFFER_SIZES[4] = { "512", "1024", "2048", "4096" };
    };
}