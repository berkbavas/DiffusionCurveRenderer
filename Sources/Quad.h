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
        unsigned int mVAO;
        unsigned int mVBO;
        static const float VERTICES[24];
    };
}
