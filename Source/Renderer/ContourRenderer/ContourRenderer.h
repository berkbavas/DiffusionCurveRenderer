#pragma once

#include "Core/Constants.h"
#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Curve/Spline.h"
#include "Renderer/Base/Blitter.h"
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
        void SetFramebufferSize(int size);

        void Render(QOpenGLFramebufferObject* target, bool clearTarget = false);
        void RenderCurve(QOpenGLFramebufferObject* target, CurvePtr curve, bool clearTarget = false);

      private:
        void RenderCurve(CurvePtr curve);

        Shader* mBezierShader;
        Interval* mInterval;

        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        QOpenGLFramebufferObjectFormat mMultisampleFramebufferFormat;

        std::unique_ptr<QOpenGLFramebufferObject> mFramebuffer{ nullptr };
        std::unique_ptr<QOpenGLFramebufferObject> mMultisampleFramebuffer{ nullptr };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
        DEFINE_MEMBER_PTR(Blitter, Blitter);
    };
}