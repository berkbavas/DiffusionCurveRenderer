#pragma once

#include "Manager.h"
#include "Quad.h"
#include "ShaderManager.h"
#include "ViewModeCamera.h"

#include <QOpenGLExtraFunctions>

#include <opencv2/core/mat.hpp>

namespace DiffusionCurveRenderer
{
    class BitmapRenderer : public Manager, protected QOpenGLExtraFunctions
    {
    private:
        explicit BitmapRenderer(QObject* parent = nullptr);

    public:
        static BitmapRenderer* Instance();

        bool Init() override;
        void Render();
        void SetData(cv::Mat image, GLenum format);
        void Resize(int w, int h);
        void SetPixelRatio(float newPixelRatio);

    private:
        ShaderManager* mShaderManager;
        ViewModeCamera* mCamera;
        Quad* mQuad;

        unsigned int mTexture;
        int mTextureWidth;
        int mTextureHeight;

        int mScreenWidth;
        int mScreeHeight;
        float mPixelRatio;
        QMatrix4x4 mImageTransformation;
    };
}
