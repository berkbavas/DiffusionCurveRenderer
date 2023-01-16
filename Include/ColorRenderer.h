#pragma once

#include "RendererBase.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

namespace DiffusionCurveRenderer
{
    class ColorRenderer : public RendererBase
    {
    public:
        ColorRenderer();

        void Render(QOpenGLFramebufferObject* draw);
    };
}

