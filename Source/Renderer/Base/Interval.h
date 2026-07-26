#pragma once

#include "Util/Macros.h"

#include <QOpenGLExtraFunctions>
#include <QVector>

namespace DiffusionCurveRenderer
{
    class Interval : protected QOpenGLExtraFunctions
    {
      public:
        Interval(float Start, float End, int Size);
        ~Interval();

        void Bind();
        void Render();
        void Release();
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