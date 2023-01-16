#include "Framebuffer.h"

DiffusionCurveRenderer::Framebuffer::Framebuffer(int width, int height)
    : mWidth(width)
    , mHeight(height)
{
    initializeOpenGLFunctions();

    glGenTextures(1, &mRenderRelatedIds[MULTISAMPLING_TEXTURE]);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mRenderRelatedIds[MULTISAMPLING_TEXTURE]);
    {
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, mWidth, mHeight, GL_TRUE);

    glGenRenderbuffers(1, &mRenderRelatedIds[MULTISAMPLING_COLOR_RBO]);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderRelatedIds[MULTISAMPLING_COLOR_RBO]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, mWidth, mHeight);
    glGenRenderbuffers(1, &mRenderRelatedIds[MULTISAMPLING_DEPTH_RBO]);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderRelatedIds[MULTISAMPLING_DEPTH_RBO]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, mWidth, mHeight);

    glGenFramebuffers(1, &mRenderRelatedIds[MULTISAMPLING_FBO]);
    glBindFramebuffer(GL_FRAMEBUFFER, mRenderRelatedIds[MULTISAMPLING_FBO]);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mRenderRelatedIds[MULTISAMPLING_TEXTURE], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderRelatedIds[MULTISAMPLING_COLOR_RBO]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRenderRelatedIds[MULTISAMPLING_DEPTH_RBO]);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        qCritical() << "Framebuffer is could not be created.";
    }
}

DiffusionCurveRenderer::Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &mRenderRelatedIds[MULTISAMPLING_FBO]);
    glDeleteFramebuffers(1, &mRenderRelatedIds[MULTISAMPLING_COLOR_RBO]);
    glDeleteFramebuffers(1, &mRenderRelatedIds[MULTISAMPLING_DEPTH_RBO]);
    glDeleteFramebuffers(1, &mRenderRelatedIds[MULTISAMPLING_TEXTURE]);
}

void DiffusionCurveRenderer::Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mRenderRelatedIds[MULTISAMPLING_FBO]);
}

unsigned int DiffusionCurveRenderer::Framebuffer::GetTexture() const
{
    return mRenderRelatedIds[MULTISAMPLING_TEXTURE];
}

int DiffusionCurveRenderer::Framebuffer::GetWidth() const
{
    return mWidth;
}

int DiffusionCurveRenderer::Framebuffer::GetHeight() const
{
    return mHeight;
}