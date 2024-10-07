#pragma once

#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

namespace DiffusionCurveRenderer
{
    class Quad : protected QOpenGLExtraFunctions
    {
      public:
        Quad();

        void Render();

      private:
        unsigned int mVertexArray;
        unsigned int mVertexBuffer;
    };
}
