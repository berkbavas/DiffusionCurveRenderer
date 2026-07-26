#pragma once

#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Util/Macros.h"

#include <opencv2/core/mat.hpp>

#include <QOpenGLExtraFunctions>

namespace DiffusionCurveRenderer
{
    class BitmapRenderer : QOpenGLExtraFunctions
    {
      public:
        BitmapRenderer();

        void Render();
        void SetImage(cv::Mat Image, GLenum InternalFormat, GLenum ExternalFormat);

      private:
        GLuint mTexture{ 0 };
        int mTextureWidth;
        int mTextureHeight;
        Quad* mQuad;
        Shader* mBitmapShader;

        QMatrix4x4 mImageTransformation;
        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
    };
}
