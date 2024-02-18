#pragma once

#include "RendererBase.h"

namespace DiffusionCurveRenderer
{
    class ContourRenderer : public RendererBase
    {
    public:
        ContourRenderer();

        void Render(QOpenGLFramebufferObject* target, bool clearTarget = true);
        void Render(QOpenGLFramebufferObject* target, Bezier* curve, bool clearTarget = true);
    };
}
