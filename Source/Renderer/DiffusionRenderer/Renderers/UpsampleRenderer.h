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

        void Upsample(const QVector<QOpenGLFramebufferObject*>& Downsamples);
        QOpenGLFramebufferObject* GetResult() const { return mUpsampleFramebuffers.first(); }
        void SetFramebufferSize(int size);

      private:
        void BlitSourceFramebuffer(QOpenGLFramebufferObject* pSource);
        void Upsample(QOpenGLFramebufferObject* pTarget, QOpenGLFramebufferObject* pTemporary, QOpenGLFramebufferObject* pSource, QOpenGLFramebufferObject* pConstraint);

        Quad* mQuad;
        Shader* mUpsampleShader;
        Shader* mJacobiShader;
        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        QVector<QOpenGLFramebufferObject*> mUpsampleFramebuffers;
        QVector<QOpenGLFramebufferObject*> mTemporaryFramebuffers;

        DEFINE_MEMBER(int, SmoothIterations, DEFAULT_SMOOTH_ITERATIONS);
    };
}