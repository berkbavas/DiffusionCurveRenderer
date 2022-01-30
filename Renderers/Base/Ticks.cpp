#include "Ticks.h"

Ticks::Ticks(float startingPoint, float endPoint, int size)
    : mStartingPoint(startingPoint)
    , mEndPoint(endPoint)
    , mSize(size)
    , mTicksDelta((mEndPoint - mStartingPoint) / mSize)
{}

Ticks::~Ticks()
{
    mTicksVertexArray.destroy();
    mTicksBuffer.destroy();
}

void Ticks::create()
{
    initializeOpenGLFunctions();

    // Ticks
    mTicks = QVector<float>(mSize, 0.0);
    for (int i = 0; i < mSize; ++i)
        mTicks[i] = mStartingPoint + i * (mEndPoint - mStartingPoint) / mSize;

    // VAO and VBO
    mTicksVertexArray.create();
    mTicksVertexArray.bind();

    mTicksBuffer.create();
    mTicksBuffer.bind();

    mTicksBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mTicksBuffer.allocate(mTicks.constData(), mSize * sizeof(GLfloat));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    mTicksBuffer.release();
    mTicksVertexArray.release();
}

void Ticks::bind()
{
    mTicksVertexArray.bind();
}

void Ticks::release()
{
    mTicksVertexArray.release();
}

float Ticks::ticksDelta() const
{
    return mTicksDelta;
}

int Ticks::size() const
{
    return mSize;
}

float Ticks::startingPoint() const
{
    return mStartingPoint;
}

float Ticks::endPoint() const
{
    return mEndPoint;
}
