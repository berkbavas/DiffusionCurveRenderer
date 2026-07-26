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
    {
        return;
    }

    mBitmapShader->Bind();
    mBitmapShader->SetUniformValue("uProjection", mCamera->GetProjectionMatrix());
    mBitmapShader->SetUniformValue("uTransformation", mImageTransformation);
    mBitmapShader->SetSampler("uSourceTexture", 0, mTexture);
    mQuad->Render();
    mBitmapShader->Release();
}

void DiffusionCurveRenderer::BitmapRenderer::SetImage(cv::Mat Image, GLenum InternalFormat, GLenum ExternalFormat)
{
    if (mTexture)
    {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }

    if (Image.empty())
    {
        LOG_FATAL("BitmapRenderer::SetImage: Image is empty. Returning...");
        return;
    }

    LOG_DEBUG("BitmapRenderer::SetImage: width = {}, height = {}, channels = {}, step = {}", Image.cols, Image.rows, Image.channels(), Image.step & 0xffffff);

    mTextureWidth = Image.cols;
    mTextureHeight = Image.rows;

    const auto Scale = 0.5 * std::min(mCamera->GetWidth() / float(mTextureWidth), mCamera->GetHeight() / float(mTextureHeight));

    mImageTransformation.setToIdentity();
    mImageTransformation.setColumn(3, QVector4D(0.5 * mCamera->GetWidth(), 0.5 * mCamera->GetHeight(), 0, 1));
    mImageTransformation.scale(QVector3D(mTextureWidth * Scale, mTextureHeight * Scale, 1));

    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // OpenCV and OpenGL stores images based on different alignments.
    // Following two calls solves this alignment issue.
    // Source: https://stackoverflow.com/questions/53558514/opengl-render-to-a-opencv-mat-as-texture-and-reuse-it-in-opencv
    // Use fast 4-byte alignment (default anyway) if possible
    glPixelStorei(GL_UNPACK_ALIGNMENT, (Image.step & 3) ? 1 : 4);

    // Set length of one complete row in data (doesn't need to equal image.cols)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, Image.step / Image.elemSize());

    glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Image.cols, Image.rows, 0, ExternalFormat, GL_UNSIGNED_BYTE, Image.ptr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}
