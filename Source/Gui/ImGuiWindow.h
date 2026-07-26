#pragma once

#include "Curve/Spline.h"
#include "Structs/Enums.h"
#include "Util/Macros.h"

#include <QObject>
#include <QStringList>
#include <QVariant>

namespace DiffusionCurveRenderer
{
    class CurveContainer;
    class RendererManager;
    class OrthographicCamera;

    class ImGuiWindow : public QObject
    {
        Q_OBJECT
      public:
        explicit ImGuiWindow(QObject* pParent);

        void Initialize();
        void Draw();

        void SetSelectedCurve(CurvePtr pSelectedCurve);
        void SetSelectedControlPoint(ControlPointPtr pPoint);
        void SetSelectedColorPoint(ColorPointPtr pPoint);

        void SetWorkMode(WorkMode WorkMode);
        void SetVectorizationStage(VectorizationStage Stage);

        int GetGaussianStackLayer() const { return mGaussianStackLayer; }
        int GetEdgeStackLayer() const { return mEdgeStackLayer; }

        void SetVectorizationViewOption(VectorizationViewOption Option);
        void SetRenderMode(RenderMode Mode, bool On);

        void AddRecentFile(const QString& Path);

      signals:
        void SelectedCurveChanged(CurvePtr pSelectedCurve);
        void SelectedControlPointChanged(ControlPointPtr pPoint);
        void SelectedColorPointChanged(ColorPointPtr pPoint);
        void ClearCanvas();

        void RenderModesChanged(RenderModes Modes);
        void UseMultisampleFramebufferChanged(bool Value);

        void ImportXml(const QString& Path);
        void SaveAsPng(const QString& Path);
        void ImportJson(const QString& Path);
        void ExportAsJson(const QString& Path);

        // Vectorization
        void WorkModeChanged(WorkMode WorkMode);
        void VectorizationViewOptionChanged(VectorizationViewOption Option);
        void GaussianStackLayerChanged(int Layer);
        void EdgeStackLayerChanged(int Layer);
        void LoadImage(const QString& Path);
        void Vectorize(VectorizationCurveType CurveType, int EdgeLevel);
        void ShowColorPointHandlesChanged(bool Value);

        // New signals for enhanced features
        void DuplicateCurve(CurvePtr pCurve);
        void BackgroundColorChanged(const QVector4D& Color);
        void ShowGridChanged(bool Show);
        void ZoomToFit();
        void ResetView();
        void ThemeChanged(ImGuiTheme Theme);

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
        void DrawViewSettings();
        void DrawAboutPopup();
        void DrawShortcutsPopup();
        void SetGaussianStackLayer(int Layer);
        void ApplyTheme(ImGuiTheme Theme);

        CurvePtr mSelectedCurve{ nullptr };
        ControlPointPtr mSelectedControlPoint{ nullptr };
        ColorPointPtr mSelectedColorPoint{ nullptr };

        // Render settings
        float mGlobalContourThickness;
        float mGlobalDiffusionWidth;
        float mGlobalDiffusionGap;
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

        // New UI state
        ImGuiTheme mCurrentTheme{ ImGuiTheme::Dark };
        QVector4D mBackgroundColor{ 0.2f, 0.2f, 0.2f, 1.0f };
        bool mShowGrid{ false };
        float mGridSpacing{ 50.0f };
        bool mShowAboutPopup{ false };
        bool mShowShortcutsPopup{ false };
        QStringList mRecentFiles;
        static constexpr int MAX_RECENT_FILES = 10;

        DEFINE_MEMBER(float, VectorizationProgress, 0.0f); // [0,1]
        DEFINE_MEMBER(int, MaximumGaussianStackLayer, 10);
        DEFINE_MEMBER(int, MaximumEdgeStackLayer, 10);

        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
        DEFINE_MEMBER_PTR(RendererManager, RendererManager);
        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER(bool, ImageLoaded, false);

        static constexpr const char* FRAME_BUFFER_SIZES[3] = { "1024", "2048", "4096" };
    };
}