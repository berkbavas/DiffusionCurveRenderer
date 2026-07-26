#include "Interval.h"

DiffusionCurveRenderer::Interval::Interval(float Start, float End, int Size)
    : mStart(Start)
    , mEnd(End)
    , mSize(Size)
{
    initializeOpenGLFunctions();

    mPoints = QVector<float>(mSize, 0.0);

    for (int i = 0; i < mSize; ++i)
    {
        mPoints[i] = mStart + (mEnd - mStart) * i / mSize;
    }

    glGenVertexArrays(1, &mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mPoints.size() * sizeof(float), mPoints.constData(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

DiffusionCurveRenderer::Interval::~Interval()
{
    if (mVertexArray)
    {
        glDeleteVertexArrays(1, &mVertexArray);
        mVertexArray = 0;
    }

    if (mVertexBuffer)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        mVertexBuffer = 0;
    }
}

void DiffusionCurveRenderer::Interval::Bind()
{
    glBindVertexArray(mVertexArray);
}

void DiffusionCurveRenderer::Interval::Render()
{
    glDrawArrays(GL_POINTS, 0, mPoints.size());
}

void DiffusionCurveRenderer::Interval::Release()
{
    glBindVertexArray(0);
}

float DiffusionCurveRenderer::Interval::GetDelta() const
{
    return (mEnd - mStart) / mSize;
}
