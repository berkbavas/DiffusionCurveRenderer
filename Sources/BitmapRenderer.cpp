#include "BitmapRenderer.h"

#include <opencv2/core/mat.hpp>

DiffusionCurveRenderer::BitmapRenderer::BitmapRenderer(QObject *parent)
    : Manager(parent)
    , mTexture(0)
    , mScreenWidth(1600)
    , mScreeHeight(900)
    , mPixelRatio(1.0f)
{}

DiffusionCurveRenderer::BitmapRenderer *DiffusionCurveRenderer::BitmapRenderer::Instance()
{
    static BitmapRenderer instance;
    return &instance;
}

bool DiffusionCurveRenderer::BitmapRenderer::Init()
{
    initializeOpenGLFunctions();

    mCamera = ViewModeCamera::Instance();
    mShaderManager = ShaderManager::Instance();

    mQuad = new Quad;

    return true;
}

void DiffusionCurveRenderer::BitmapRenderer::Render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mPixelRatio * mScreenWidth, mPixelRatio * mScreeHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    mShaderManager->Bind(ShaderType::BitmapShader);
    mShaderManager->SetUniformValue("projection", mCamera->GetProjection());
    mShaderManager->SetUniformValue("transformation", mImageTransformation);
    mShaderManager->SetSampler("sourceTexture", 0, mTexture);
    mQuad->Render();
    mShaderManager->Release();
}

void DiffusionCurveRenderer::BitmapRenderer::SetData(cv::Mat image, GLenum format)
{
    if (mTexture)
        glDeleteTextures(1, &mTexture);

    mTextureWidth = image.cols;
    mTextureHeight = image.rows;

    mImageTransformation.setToIdentity();
    mImageTransformation.scale(QVector3D(mTextureWidth / 2.0, mTextureHeight / 2.0, 1));
    mImageTransformation.setColumn(3, QVector4D(0, 0, 0, 1));

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, mTextureWidth, mTextureHeight, 0, format, GL_UNSIGNED_BYTE, image.ptr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void DiffusionCurveRenderer::BitmapRenderer::Resize(int w, int h)
{
    mScreenWidth = w;
    mScreeHeight = h;
}

void DiffusionCurveRenderer::BitmapRenderer::SetPixelRatio(float newPixelRatio)
{
    mPixelRatio = newPixelRatio;
}
