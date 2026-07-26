#pragma once

#include <QOpenGLExtraFunctions>

namespace DiffusionCurveRenderer
{
    class Quad : protected QOpenGLExtraFunctions
    {
      public:
        Quad();
        ~Quad();

        void Render();

      private:
        GLuint mVertexArray;
        GLuint mVertexBuffer;
    };
}
