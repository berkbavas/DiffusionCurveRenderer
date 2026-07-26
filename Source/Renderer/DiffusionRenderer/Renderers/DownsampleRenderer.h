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

        void Downsample(QOpenGLFramebufferObject* pSource);
        const QVector<QOpenGLFramebufferObject*>& GetFramebuffers() const { return mFramebuffers; }
        void SetFramebufferSize(int Size);

      private:
        void BlitSourceFramebuffer(QOpenGLFramebufferObject* pSource);
        void Downsample(QOpenGLFramebufferObject* pSource, QOpenGLFramebufferObject* pTarget);

        Quad* mQuad;
        Shader* mDownsampleShader;
        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        QVector<QOpenGLFramebufferObject*> mFramebuffers;
    };
}