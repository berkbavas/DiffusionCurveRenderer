#pragma once

#include "Core/Constants.h"
#include "Core/OrthographicCamera.h"
#include "Curve/CurveContainer.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class BlurRenderer : protected QOpenGLExtraFunctions
    {
      public:
        BlurRenderer();

        void Blur(QOpenGLFramebufferObject* framebuffer);

        QOpenGLFramebufferObject* GetResult() const { return mFramebuffer; }

        void DeleteFramebuffer();
        void CreateFramebuffer();

      private:
        void LastBlurPass(QOpenGLFramebufferObject* framebuffer);
        void Combine(QOpenGLFramebufferObject* source);

        Quad* mQuad;
        Interval* mInterval;
        Shader* mBlurShader;
        Shader* mLastBlurPassShader;

        QOpenGLFramebufferObject* mFramebuffer;
        QOpenGLFramebufferObjectFormat mFramebufferFormat;

        DEFINE_MEMBER(int, FramebufferSize, DEFAULT_FRAMEBUFFER_SIZE);

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}