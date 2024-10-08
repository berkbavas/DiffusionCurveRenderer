#pragma once

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <memory>

namespace DiffusionCurveRenderer
{
    class BlurRenderer : protected QOpenGLExtraFunctions
    {
      public:
        BlurRenderer();

        void Blur(QOpenGLFramebufferObject* target, QOpenGLFramebufferObject* source);
        void SetFramebufferSize(int size);

      private:
        void LastBlurPass(QOpenGLFramebufferObject* framebuffer);
        void Combine(QOpenGLFramebufferObject* source);

        Quad* mQuad;
        Interval* mInterval;
        Shader* mBlurShader;
        Shader* mLastBlurPassShader;

        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        std::unique_ptr<QOpenGLFramebufferObject> mFramebuffer{ nullptr };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}