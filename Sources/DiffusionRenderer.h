#pragma once

#include "RendererBase.h"

#include <QOpenGLFramebufferObjectFormat>

namespace DiffusionCurveRenderer
{
    class DiffusionRenderer : public RendererBase
    {
    public:
        DiffusionRenderer();

        void Init() override;
        void Resize(int w, int h) override;
        void SetQualityFactor(float newQualityFactor) override;
        void SetPixelRatio(float newPixelRatio) override;
        void SetSmoothIterations(int newSmoothIterations);

        void Render(QOpenGLFramebufferObject* initialFramebuffer, QOpenGLFramebufferObject* target, bool clearTarget = true);

    private:
        void Downsample(QOpenGLFramebufferObject* draw, QOpenGLFramebufferObject* read);
        void Upsample(QOpenGLFramebufferObject* draw, QOpenGLFramebufferObject* drawBuffer, QOpenGLFramebufferObject* source, QOpenGLFramebufferObject* target);
        void DrawFinalBlurCurves(QOpenGLFramebufferObject* draw);

        void DeleteFramebuffers();
        void CreateFramebuffers();

    private:
        QOpenGLFramebufferObjectFormat mDefaultFramebufferFormat;
        QVector<QOpenGLFramebufferObject*> mDownsampleFramebuffers;
        QVector<QOpenGLFramebufferObject*> mUpsampleFramebuffers;
        QVector<QOpenGLFramebufferObject*> mTemporaryFrameBuffers;
        GLenum* mDrawBuffers;

        int mSmoothIterations;
    };
}
