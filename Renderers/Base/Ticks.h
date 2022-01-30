#ifndef TICKS_H
#define TICKS_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

class Ticks : protected QOpenGLFunctions
{
public:
    Ticks(float startingPoint, float endPoint, int size);
    ~Ticks();

    void create();
    void bind();
    void release();

    float startingPoint() const;
    float endPoint() const;
    int size() const;
    float ticksDelta() const;

private:
    QOpenGLVertexArrayObject mTicksVertexArray;
    QOpenGLBuffer mTicksBuffer;

    QVector<float> mTicks;

    const float mStartingPoint;
    const float mEndPoint;
    const int mSize;
    const float mTicksDelta;
};

#endif // TICKS_H
