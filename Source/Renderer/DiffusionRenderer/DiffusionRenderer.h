#pragma once

#include "Core/OrthographicCamera.h"
#include "Curve/CurveContainer.h"
#include "Renderer/Base/MultisampleFramebuffer.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <map>

namespace DiffusionCurveRenderer
{
    class ColorRenderer;
    class DownsampleRenderer;
    class UpsampleRenderer;
    class BlurRenderer;

    class DiffusionRenderer : protected QOpenGLExtraFunctions
    {
      public:
        DiffusionRenderer() = default;

        void Initialize();
        void Render(QOpenGLFramebufferObject* framebuffer);

        QOpenGLFramebufferObject* GetResult() const;

        int GetSmoothIterations() const;

        void SetFramebufferSize(int newSize);
        void SetSmoothIterations(int smoothIterations);

      private:
        void ClearFramebuffer(QOpenGLFramebufferObject* framebuffer);

        ColorRenderer* mColorRenderer;
        DownsampleRenderer* mDownsampleRenderer;
        UpsampleRenderer* mUpsampleRenderer;
        BlurRenderer* mBlurRenderer;

        int mFramebufferSize{ DEFAULT_FRAMEBUFFER_SIZE };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}