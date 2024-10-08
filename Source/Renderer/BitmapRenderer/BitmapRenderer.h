#pragma once

#include "Core/OrthographicCamera.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"
#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <opencv2/core/mat.hpp>

namespace DiffusionCurveRenderer
{
    class BitmapRenderer : QOpenGLExtraFunctions
    {
      public:
        BitmapRenderer();

        void Render();
        void SetImage(cv::Mat image, GLenum internalFormat, GLenum externalFormat);

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
