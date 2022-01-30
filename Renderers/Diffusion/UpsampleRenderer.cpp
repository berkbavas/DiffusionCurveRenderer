#include "UpsampleRenderer.h"

#include <Helper.h>

UpsampleRenderer::UpsampleRenderer()
    : mShader(nullptr)
    , mQuads(nullptr)
    , mInit(false)
{}

UpsampleRenderer::~UpsampleRenderer()
{
    if (mShader)
        mShader->deleteLater();
    if (mQuads)
        delete mQuads;

    mShader = nullptr;
    mQuads = nullptr;
}

bool UpsampleRenderer::init()
{
    initializeOpenGLFunctions();

    mShader = new QOpenGLShaderProgram;

    if (!mShader->addShaderFromSourceCode(QOpenGLShader::Vertex, Helper::getBytes(":/Resources/Shaders/Diffusion/Upsample/VertexShader.vert"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Fragment, Helper::getBytes(":/Resources/Shaders/Diffusion/Upsample/FragmentShader.frag"))
        || !mShader->link() || !mShader->bind()) {
        qCritical() << mShader->log();
        return false;
    }

    mSourceTextureLocation = mShader->uniformLocation("sourceTexture");
    mTargetTextureLocation = mShader->uniformLocation("targetTexture");

    qDebug() << "UpsampleRenderer Locations:" << mSourceTextureLocation << mTargetTextureLocation;

    mShader->bindAttributeLocation("vs_Position", 0);
    mShader->bindAttributeLocation("vs_TextureCoords", 1);
    mShader->release();

    mQuads = new Quads;
    mQuads->create();

    return mInit = true;
}

void UpsampleRenderer::render(GLuint sourceTexture, GLuint targetTexture)
{
    mShader->bind();

    mShader->setUniformValue(mSourceTextureLocation, 0);
    mShader->setUniformValue(mTargetTextureLocation, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    mQuads->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    mQuads->release();

    mShader->release();
}
