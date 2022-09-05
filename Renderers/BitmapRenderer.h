#ifndef BITMAPRENDERER_H
#define BITMAPRENDERER_H

#include "Manager.h"
#include "Quad.h"
#include "ShaderManager.h"
#include "ViewModeCamera.h"

#include <QOpenGLExtraFunctions>

#include <opencv2/core/mat.hpp>

class BitmapRenderer : public Manager, protected QOpenGLExtraFunctions
{
private:
    explicit BitmapRenderer(QObject *parent = nullptr);

public:
    static BitmapRenderer *instance();

    bool init() override;
    void render();
    void setData(cv::Mat image, GLenum format);
    void resize(int w, int h);
    void setPixelRatio(float newPixelRatio);

private:
    ShaderManager *mShaderManager;
    ViewModeCamera *mCamera;
    Quad *mQuad;

    unsigned int mTexture;
    int mTextureWidth;
    int mTextureHeight;

    int mScreenWidth;
    int mScreeHeight;
    float mPixelRatio;
    QMatrix4x4 mImageTransformation;
};

#endif // BITMAPRENDERER_H