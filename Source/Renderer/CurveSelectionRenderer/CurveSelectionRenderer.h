#pragma once

#include "Core/Constants.h"
#include "Core/OrthographicCamera.h"
#include "Curve/CurveContainer.h"
#include "Curve/Spline.h"
#include "Renderer/Base/Interval.h"
#include "Renderer/Base/Shader.h"
#include "Renderer/CurveSelectionRenderer/CurveSelectionFramebuffer.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class CurveSelectionRenderer : protected QOpenGLExtraFunctions
    {
      public:
        CurveSelectionRenderer();

        void Render();
        CurveQueryInfo Query(const QPoint& queryPoint);

        void Resize(int width, int height);

      private:
        Shader* mCurveSelectionShader;
        Interval* mInterval;

        CurveSelectionFramebufferPtr mFramebuffer{ nullptr };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);

        DEFINE_MEMBER(float, CurveSelectionWidth, DEFAULT_CURVE_SELECTION_WIDTH);
    };
}