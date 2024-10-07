#pragma once

#include "Core/Constants.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class UpsampleRenderer : protected QOpenGLExtraFunctions
    {
      public:
        UpsampleRenderer();

        void Upsample(QVector<QOpenGLFramebufferObject*> downsampleFramebuffers);

        QOpenGLFramebufferObject* GetResult() const { return mUpsampleFramebuffers.first(); }

        void DeleteFramebuffers();
        void CreateFramebuffers();

      private:
        void BlitSourceFramebuffer(QOpenGLFramebufferObject* source);
        void Upsample(QOpenGLFramebufferObject* target, QOpenGLFramebufferObject* temporary, QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* constraint);

        Quad* mQuad;

        Shader* mUpsampleShader;
        Shader* mJacobiShader;

        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        QVector<QOpenGLFramebufferObject*> mUpsampleFramebuffers;
        QVector<QOpenGLFramebufferObject*> mTemporaryFramebuffers;

        DEFINE_MEMBER(int, SmoothIterations, DEFAULT_SMOOTH_ITERATIONS);
        DEFINE_MEMBER(int, FramebufferSize, DEFAULT_FRAMEBUFFER_SIZE);
    };
}