#include "Points.h"

DiffusionCurveRenderer::Points::Points()
    : mStart(0)
    , mEnd(1)
    , mSize(200)
{
    initializeOpenGLFunctions();

    mDelta = (mEnd - mStart) / mSize;

    // Ticks
    mPoints = QVector<float>(mSize, 0.0);
    for (int i = 0; i < mSize; ++i)
        mPoints[i] = mStart + i * (mEnd - mStart) / mSize;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, mPoints.size() * sizeof(float), mPoints.constData(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
}

DiffusionCurveRenderer::Points::~Points()
{
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
}

void DiffusionCurveRenderer::Points::Bind()
{
    glBindVertexArray(mVAO);
}

void DiffusionCurveRenderer::Points::Release()
{
    glBindVertexArray(0);
}

float DiffusionCurveRenderer::Points::GetDelta() const
{
    return mDelta;
}

int DiffusionCurveRenderer::Points::GetSize() const
{
    return mSize;
}