#pragma once

#include "Core/Constants.h"
#include "Core/OrthographicCamera.h"
#include "Curve/CurveContainer.h"
#include "Renderer/Base/MultisampleFramebuffer.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Renderer/CurveSelectionRenderer/CurveSelectionRenderer.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <map>

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
        void RenderContours(QVector4D* globalColorOption);
        void RenderForCurveSelection();
        void RenderCurve(CurvePtr curve);

        BitmapRenderer* GetBitmapRenderer() { return mBitmapRenderer; }

        int GetFramebufferSize() const { return mFramebufferSize; };
        void SetFramebufferSize(int newSize);

        int GetSmoothIterations();
        void SetSmoothIterations(int smoothIterations);

        CurveQueryInfo Query(const QPoint& queryPoint);

      private:
        void CreateFramebuffers();
        void DeleteFramebuffers();
        void BlitFromResult(QOpenGLFramebufferObject* result);

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);

        Shader* mScreenShader;
        Quad* mQuad;

        ContourRenderer* mContourRenderer;
        DiffusionRenderer* mDiffusionRenderer;
        CurveSelectionRenderer* mCurveSelectionRenderer;
        BitmapRenderer* mBitmapRenderer;

        QOpenGLFramebufferObjectFormat mContourFramebufferFormat;
        QOpenGLFramebufferObjectFormat mDiffusionFramebufferFormat;

        QOpenGLFramebufferObject* mContourFramebuffer{ nullptr };
        QOpenGLFramebufferObject* mDiffusionFramebuffer{ nullptr };

        int mFramebufferSize{ DEFAULT_FRAMEBUFFER_SIZE };
    };
}