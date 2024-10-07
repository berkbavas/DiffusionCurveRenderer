#pragma once

#include "Core/Constants.h"
#include "Core/OrthographicCamera.h"
#include "Curve/CurveContainer.h"
#include "Curve/Spline.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Shader.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class ContourRenderer : protected QOpenGLExtraFunctions
    {
      public:
        ContourRenderer();

        void Render(QOpenGLFramebufferObject* framebuffer, QVector4D* globalColorOption);
        void RenderCurve(QOpenGLFramebufferObject* framebuffer, CurvePtr curve);

      private:
        void RenderCurve(CurvePtr curve);

        Shader* mBezierShader;
        Interval* mInterval;

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}