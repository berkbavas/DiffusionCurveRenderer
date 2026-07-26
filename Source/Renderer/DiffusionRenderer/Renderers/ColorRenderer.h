#pragma once

#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Shader.h"

#include <memory>

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>


namespace DiffusionCurveRenderer
{
    class ColorRenderer : protected QOpenGLExtraFunctions
    {
      public:
        ColorRenderer();

        void Render(QOpenGLFramebufferObject* pTarget);
        void SetFramebufferSize(int Size);

      private:
        void RenderPrivate(QOpenGLFramebufferObject* pTarget);
        void SetUniforms(BezierPtr pCurve);
        void BlitFramebuffer(QOpenGLFramebufferObject* pSource, QOpenGLFramebufferObject* pTarget);

        Interval* mInterval;
        Shader* mColorShader;
        QOpenGLFramebufferObjectFormat mMultisampleFramebufferFormat;
        std::unique_ptr<QOpenGLFramebufferObject> mMultisampleFramebuffer{ nullptr };

        DEFINE_MEMBER(bool, UseMultisampleFramebuffer, false);
        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}