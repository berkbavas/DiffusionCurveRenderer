#pragma once

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/Base/MultisampleFramebuffer.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Renderer/CurveSelectionRenderer/CurveSelectionRenderer.h"

#include <map>
#include <memory>

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QVector4D>

namespace DiffusionCurveRenderer
{
    class ContourRenderer;
    class DiffusionRenderer;
    class BitmapRenderer;

    class RendererManager : protected QOpenGLExtraFunctions
    {
      public:
        RendererManager() = default;

        void Initialize();
        void Resize(int Width, int Height);
        void Clear();
        void RenderDiffusion();
        void RenderContours();
        void RenderForCurveSelection();
        void RenderCurve(CurvePtr pCurve);
        void Save(const QString& Path, RenderModes RenderModes);
        BitmapRenderer* GetBitmapRenderer() { return mBitmapRenderer; }
        void SetFramebufferSize(int Size);
        void SetSmoothIterations(int SmoothIterations);
        void SetUseMultisampleFramebuffer(bool Value);
        void SetBackgroundColor(const QVector4D& Color) { mBackgroundColor = Color; }
        int GetSmoothIterations() const;
        int GetFramebufferSize() const { return mFramebufferSize; };
        const QVector4D& GetBackgroundColor() const { return mBackgroundColor; }
        CurveQueryInfo Query(const QPoint& QueryPoint);

      private:
        ContourRenderer* mContourRenderer;
        DiffusionRenderer* mDiffusionRenderer;
        CurveSelectionRenderer* mCurveSelectionRenderer;
        BitmapRenderer* mBitmapRenderer;
        int mFramebufferSize{ DEFAULT_FRAMEBUFFER_SIZE };
        QVector4D mBackgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        std::unique_ptr<QOpenGLFramebufferObject> mSaveFramebuffer{ nullptr };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}