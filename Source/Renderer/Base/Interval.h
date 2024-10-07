#pragma once

#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <QVector>

namespace DiffusionCurveRenderer
{
    class Interval : protected QOpenGLExtraFunctions
    {
      public:
        Interval(float start, float end, int size);

        void Bind();
        void Render();
        void Release();
        void Destroy();
        float GetDelta() const;

      private:
        QVector<float> mPoints;
        float mStart;
        float mEnd;
        int mSize;
        GLuint mVertexArray{ 0 };
        GLuint mVertexBuffer{ 0 };
    };

}