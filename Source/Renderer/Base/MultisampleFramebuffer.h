#pragma once

#include <QOpenGLFunctions_4_5_Core>

namespace DiffusionCurveRenderer
{
    class MultisampleFramebuffer : protected QOpenGLFunctions_4_5_Core
    {
      public:
        MultisampleFramebuffer();

        void Clear();
        void Bind();

        GLuint GetHandle() const { return mFramebuffer; }
        GLuint GetTexture() const { return mTexture; }

      private:
        GLuint mFramebuffer;
        GLuint mTexture;
    };

}