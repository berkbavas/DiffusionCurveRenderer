#pragma once

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Curve/Spline.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Shader.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <memory>

namespace DiffusionCurveRenderer
{
    class ContourRenderer : QOpenGLExtraFunctions
    {
      public:
        ContourRenderer() = default;

        void Initialize();

        void Render();
        void RenderCurve(CurvePtr curve);

      private:
        void RenderCurveInner(CurvePtr curve);

        Shader* mBezierShader;
        Interval* mInterval;

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}