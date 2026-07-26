#include "MultisampleFramebuffer.h"

#include "Core/Constants.h"
#include "Util/Logger.h"

DiffusionCurveRenderer::MultisampleFramebuffer::MultisampleFramebuffer()
{
    initializeOpenGLFunctions();

    // Generate framebuffer
    glGenFramebuffers(1, &mFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Generate texture
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, DEFAULT_FRAMEBUFFER_SIZE, DEFAULT_FRAMEBUFFER_SIZE, GL_FALSE);

    // Bind texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        DCR_EXIT_FAILURE("Could not create multisampled framebuffer!");
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DiffusionCurveRenderer::MultisampleFramebuffer::Clear()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glViewport(0, 0, DEFAULT_FRAMEBUFFER_SIZE, DEFAULT_FRAMEBUFFER_SIZE);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DiffusionCurveRenderer::MultisampleFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
}
