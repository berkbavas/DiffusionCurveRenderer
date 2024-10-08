#pragma once

#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
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
        void Render(QOpenGLFramebufferObject* target);

        int GetSmoothIterations() const;

        void SetFramebufferSize(int newSize);
        void SetSmoothIterations(int smoothIterations);
        void SetUseMultisampleFramebuffer(bool val);

      private:
        ColorRenderer* mColorRenderer;
        DownsampleRenderer* mDownsampleRenderer;
        UpsampleRenderer* mUpsampleRenderer;
        BlurRenderer* mBlurRenderer;

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}