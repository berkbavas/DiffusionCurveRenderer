#pragma once

#include "Common.h"

#include "ColorRenderer.h"
#include "ContourRenderer.h"
#include "DiffusionRenderer.h"
#include "Manager.h"
#include "Points.h"
#include "Quad.h"

#include <QOpenGLFramebufferObjectFormat>

namespace DiffusionCurveRenderer
{
    class ShaderManager;
    class CurveManager;

    class RendererManager : public Manager, protected QOpenGLExtraFunctions
    {
        explicit RendererManager(QObject* parent = nullptr);

    public:
        static RendererManager* Instance();

        bool Init() override;
        void Render();
        void Resize(int width, int height);
        void SetRenderMode(RenderMode newRenderMode);
        void SetSmoothIterations(int newSmoothIterations);
        void SetQualityFactor(float newQualityFactor);
        void SetPixelRatio(float newPixelRatio);
        void Save(const QString& path);

    private:
        void CreateFramebuffers();
        void DeleteFramebuffers();

    private:
        CurveManager* mCurveManager;

        ContourRenderer* mContourRenderer;
        ColorRenderer* mColorRenderer;
        DiffusionRenderer* mDiffusionRenderer;

        Points* mPoints;
        Quad* mQuad;

        QOpenGLFramebufferObjectFormat mInitialFramebufferFormat;
        QOpenGLFramebufferObjectFormat mFinalFramebufferFormat;            // For mFinalFramebuffer
        QOpenGLFramebufferObjectFormat mFinalMultisampleFramebufferFormat; // For mFinalMultisampleFramebuffer

        QOpenGLFramebufferObject* mInitialFramebuffer;          // Contains initial color and blur info
        QOpenGLFramebufferObject* mFinalFramebuffer;            // Final single sampled result for colors and blur in order to save the image
        QOpenGLFramebufferObject* mFinalMultisampleFramebuffer; // We render contours to this FBO
        GLenum* mDrawBuffers;

        RenderMode mRenderMode;
        int mSmoothIterations;
        float mQualityFactor;
        int mWidth;
        int mHeight;
        float mPixelRatio;

        bool mSave;
        QString mSavePath;
    };
}
