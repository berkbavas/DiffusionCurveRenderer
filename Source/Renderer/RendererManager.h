#pragma once

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Blitter.h"
#include "Renderer/Base/MultisampleFramebuffer.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Renderer/CurveSelectionRenderer/CurveSelectionRenderer.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <map>
#include <memory>

namespace DiffusionCurveRenderer
{
    class ContourRenderer;
    class DiffusionRenderer;
    class CurveSelectionRenderer;
    class BitmapRenderer;

    class RendererManager : protected QOpenGLExtraFunctions
    {
      public:
        RendererManager() = default;

        void Initialize();
        void Resize(int width, int height);

        void Clear();
        void RenderDiffusion();
        void RenderContours();
        void RenderForCurveSelection();
        void RenderCurve(CurvePtr curve);

        BitmapRenderer* GetBitmapRenderer() { return mBitmapRenderer; }

        void SetFramebufferSize(int size);
        void SetSmoothIterations(int smoothIterations);
        void SetUseMultisampleFramebuffer(bool val);

        int GetSmoothIterations() const;
        int GetFramebufferSize() const { return FramebufferSize; };

        CurveQueryInfo Query(const QPoint& queryPoint);

      private:
        ContourRenderer* mContourRenderer;
        DiffusionRenderer* mDiffusionRenderer;
        CurveSelectionRenderer* mCurveSelectionRenderer;
        BitmapRenderer* mBitmapRenderer;

        Blitter* mBlitter;
        Quad* mQuad;

        QOpenGLFramebufferObjectFormat mContourFramebufferFormat;
        QOpenGLFramebufferObjectFormat mDiffusionFramebufferFormat;

        std::unique_ptr<QOpenGLFramebufferObject> mDiffusionFramebuffer{ nullptr };
        std::unique_ptr<QOpenGLFramebufferObject> mContourFramebuffer{ nullptr };

        int FramebufferSize{ DEFAULT_FRAMEBUFFER_SIZE };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}