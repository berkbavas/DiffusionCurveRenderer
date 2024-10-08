#pragma once

#include <QOpenGLExtraFunctions>

namespace DiffusionCurveRenderer
{
    class Quad : protected QOpenGLExtraFunctions
    {
      public:
        Quad();

        void Render();

      private:
        GLuint mVertexArray;
        GLuint mVertexBuffer;
    };
}
