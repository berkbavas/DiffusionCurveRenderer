#pragma once

#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Shader.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <memory>

namespace DiffusionCurveRenderer
{
    class ColorRenderer : protected QOpenGLExtraFunctions
    {
      public:
        ColorRenderer();

        void Render(QOpenGLFramebufferObject* target);

        void SetFramebufferSize(int size);

      private:
        void RenderPrivate(QOpenGLFramebufferObject* target);
        void SetUniforms(BezierPtr curve);
        void BlitFramebuffer(QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* target);

        Interval* mInterval;
        Shader* mColorShader;

        DEFINE_MEMBER(bool, UseMultisampleFramebuffer, false);

        QOpenGLFramebufferObjectFormat mMultisampleFramebufferFormat;
        std::unique_ptr<QOpenGLFramebufferObject> mMultisampleFramebuffer{ nullptr };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}