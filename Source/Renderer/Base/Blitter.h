#pragma once

#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class Blitter : public QOpenGLFunctions
    {
      public:
        Blitter();

        void Blit(QOpenGLFramebufferObject* target, QOpenGLFramebufferObject* source, bool clearTarget = false);

      private:
        Shader* mScreenShader;
        Quad* mQuad;

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
    };
}