#pragma once

#include "Core/Constants.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class DownsampleRenderer : protected QOpenGLExtraFunctions
    {
      public:
        DownsampleRenderer();

        void Downsample(QOpenGLFramebufferObject* source);

        const QVector<QOpenGLFramebufferObject*>& GetFramebuffers() const { return mFramebuffers; }

        void SetFramebufferSize(int size);

      private:
        void BlitSourceFramebuffer(QOpenGLFramebufferObject* source);
        void Downsample(QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* target);

        Quad* mQuad;
        Shader* mDownsampleShader;
        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        QVector<QOpenGLFramebufferObject*> mFramebuffers;
    };
}