#include "BitmapRenderer.h"

#include <cmath>

DiffusionCurveRenderer::BitmapRenderer::BitmapRenderer()
{
    initializeOpenGLFunctions();

    mQuad = new Quad;

    mBitmapShader = new Shader("Bitmap Shader");
    mBitmapShader->AddPath(QOpenGLShader::Vertex, ":/Resources/Shaders/Bitmap.vert");
    mBitmapShader->AddPath(QOpenGLShader::Fragment, ":/Resources/Shaders/Bitmap.frag");
    mBitmapShader->Initialize();
}

void DiffusionCurveRenderer::BitmapRenderer::Render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mCamera->GetWidth(), mCamera->GetHeight());
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (mTexture == 0)
        return;

    mBitmapShader->Bind();
    mBitmapShader->SetUniformValue("projection", mCamera->GetProjectionMatrix());
    mBitmapShader->SetUniformValue("transformation", mImageTransformation);
    mBitmapShader->SetSampler("sourceTexture", 0, mTexture);
    mQuad->Render();
    mBitmapShader->Release();
}

void DiffusionCurveRenderer::BitmapRenderer::SetImage(cv::Mat image, GLenum internalFormat, GLenum format)
{
    if (mTexture)
    {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }

    if (image.empty())
    {
        LOG_FATAL("BitmapRenderer::SetImage: Image is empty. Returning...");
        return;
    }

    LOG_DEBUG("BitmapRenderer::SetImage: width = {}, height = {}, channels = {}", image.cols, image.rows, image.channels());

    mTextureWidth = image.cols;
    mTextureHeight = image.rows;

    float scale = 0.5 * std::min(mCamera->GetWidth() / float(mTextureWidth), mCamera->GetHeight() / float(mTextureHeight));

    mImageTransformation.setToIdentity();
    mImageTransformation.setColumn(3, QVector4D(0.5 * mCamera->GetWidth(), 0.5 * mCamera->GetHeight(), 0, 1));
    mImageTransformation.scale(QVector3D(mTextureWidth * scale, mTextureHeight * scale, 1));

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.cols, image.rows, 0, format, GL_UNSIGNED_BYTE, image.ptr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}
