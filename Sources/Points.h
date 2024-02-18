#pragma once

#include <QOpenGLExtraFunctions>

namespace DiffusionCurveRenderer
{
    class Points : protected QOpenGLExtraFunctions
    {
    public:
        Points();
        ~Points();

        void Bind();
        void Release();
        float GetDelta() const;
        int GetSize() const;

    private:
        unsigned int mVAO;
        unsigned int mVBO;

        QVector<float> mPoints;

        float mStart;
        float mEnd;
        int mSize;
        float mDelta;
    };
}
