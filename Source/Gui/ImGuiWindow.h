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

        void SetWorkMode(WorkMode workMode);
        void SetVectorizationStage(VectorizationStage stage);

        int GetGaussianStackLayer() const { return mGaussianStackLayer; }
        int GetEdgeStackLayer() const { return mEdgeStackLayer; }

        void SetVectorizationViewOption(VectorizationViewOption option);
        void SetRenderMode(RenderMode mode, bool on);

      signals:
        void SelectedCurveChanged(CurvePtr selectedCurve);
        void SelectedControlPointChanged(ControlPointPtr point);
        void SelectedColorPointChanged(ColorPointPtr point);

        void RenderModesChanged(RenderModes modes);
        void UseMultisampleFramebufferChanged(bool val);

        // Vectorization
        void WorkModeChanged(WorkMode workMode);
        void VectorizationViewOptionChanged(VectorizationViewOption option);
        void GaussianStackLayerChanged(int layer);
        void EdgeStackLayerChanged(int layer);
        void LoadImage(const QString& path);
        void Vectorize(VectorizationCurveType curveType, int edgeLevel);
        void ShowColorPointHandlesChanged(bool value);

      private:
        void DrawWorkModes();
        void DrawVectorizationViewOptions();
        void DrawCurveEditingSettings();
        void DrawMenuBar();
        void DrawHintTexts();
        void DrawRenderMode();
        void DrawCurveHeader();
        void DrawRenderSettings();
        void DrawStats();
        void SetGaussianStackLayer(int layer);

        CurvePtr mSelectedCurve{ nullptr };
        ControlPointPtr mSelectedControlPoint{ nullptr };
        ColorPointPtr mSelectedColorPoint{ nullptr };

        // Render settings
        float mGlobalContourThickness;
        float mGlobalDiffusionWidth;
        float mGlobalDiffusionGap;
        float mGlobalBlurStrength;
        int mSmoothIterations;
        int mFrambufferSize;
        int mFrambufferSizeIndex;
        bool mUseMultisampleFramebuffer{ false };

        WorkMode mWorkMode{ WorkMode::CurveEditing };
        VectorizationStage mVectorizationStage{ VectorizationStage::Initial };
        VectorizationViewOption mVectorizationViewOption{ VectorizationViewOption::ViewOriginalImage };
        int mGaussianStackLayer{ 0 };
        int mEdgeStackLayer{ 0 };

        RenderModes mRenderModes{ RenderMode::Diffusion | RenderMode::Contour };

        bool mRenderModeDiffusion{ true };
        bool mRenderModeContour{ true };

        bool mShowColorPointHandles{ true };

        VectorizationCurveType mVectorizationCurveType{ VectorizationCurveType::Bezier };

        DEFINE_MEMBER(float, VectorizationProgress, 0.0f); // [0,1]
        DEFINE_MEMBER(int, MaximumGaussianStackLayer, 10);
        DEFINE_MEMBER(int, MaximumEdgeStackLayer, 10);

        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
        DEFINE_MEMBER_PTR(RendererManager, RendererManager);
        DEFINE_MEMBER(bool, ImageLoaded, false);

        static constexpr const char* FRAME_BUFFER_SIZES[4] = { "512", "1024", "2048", "4096" };
    };
}