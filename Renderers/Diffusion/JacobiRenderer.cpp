#include "JacobiRenderer.h"

#include <Helper.h>

JacobiRenderer::JacobiRenderer()
    : mShader(nullptr)
    , mQuads(nullptr)
    , mInit(false)
{}

JacobiRenderer::~JacobiRenderer()
{
    if (mShader)
        mShader->deleteLater();
    if (mQuads)
        delete mQuads;

    mShader = nullptr;
    mQuads = nullptr;
}

bool JacobiRenderer::init()
{
    initializeOpenGLFunctions();

    mShader = new QOpenGLShaderProgram;

    if (!mShader->addShaderFromSourceCode(QOpenGLShader::Vertex, Helper::getBytes(":/Resources/Shaders/Diffusion/Jacobi/VertexShader.vert"))
        || !mShader->addShaderFromSourceCode(QOpenGLShader::Fragment, Helper::getBytes(":/Resources/Shaders/Diffusion/Jacobi/FragmentShader.frag"))
        || !mShader->link() || !mShader->bind()) {
        qCritical() << mShader->log();
        return false;
    }

    mConstrainedTextureLocation = mShader->uniformLocation("constrainedTexture");
    mTargetTextureLocation = mShader->uniformLocation("targetTexture");

    qDebug() << "JacobiRenderer Locations:" << mConstrainedTextureLocation << mTargetTextureLocation;

    mShader->bindAttributeLocation("vs_Position", 0);
    mShader->bindAttributeLocation("vs_TextureCoords", 1);
    mShader->release();

    mQuads = new Quads;
    mQuads->create();

    return mInit = true;
}

void JacobiRenderer::render(GLuint constrainedTexture, GLuint targetTexture)
{
    if (!mInit)
        return;

    mShader->bind();
    mQuads->bind();

    mShader->setUniformValue(mConstrainedTextureLocation, 0);
    mShader->setUniformValue(mTargetTextureLocation, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, constrainedTexture);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    mQuads->release();
    mShader->release();
}
