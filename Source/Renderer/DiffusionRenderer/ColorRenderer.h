#pragma once

#include "Core/OrthographicCamera.h"
#include "Curve/CurveContainer.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Shader.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class ColorRenderer : protected QOpenGLExtraFunctions
    {
      public:
        ColorRenderer();

        void Render(QOpenGLFramebufferObject* framebuffer);

      private:
        void SetUniforms(BezierPtr curve);

        Interval* mInterval;
        Shader* mColorShader;

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}