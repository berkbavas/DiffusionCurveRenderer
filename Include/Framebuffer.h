#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_4_2_Core>

namespace DiffusionCurveRenderer
{
    class Framebuffer : protected QOpenGLFunctions_4_2_Core
    {
    public:
        Framebuffer(int width, int height);
        ~Framebuffer();

        enum FBORenderTarget {
            MULTISAMPLING_FBO,
            MULTISAMPLING_TEXTURE,
            MULTISAMPLING_COLOR_RBO,
            MULTISAMPLING_DEPTH_RBO,
        };

        void Bind();
        unsigned int GetTexture() const;
        int GetWidth() const;
        int GetHeight() const;

    private:
        GLuint mRenderRelatedIds[8];

        int mWidth;
        int mHeight;
    };
}
