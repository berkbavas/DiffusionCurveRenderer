#include "Quad.h"

DiffusionCurveRenderer::Quad::Quad()
{
    initializeOpenGLFunctions();

    constexpr float VERTICES[24] = {
        -1.0f, +1.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        +1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
        -1.0f, +1.0f, 0.0f, 1.0f, // Top-left
        +1.0f, -1.0f, 1.0f, 0.0f, // Bottom-right
        +1.0f, +1.0f, 1.0f, 1.0f  // Top-right
    };

    glGenVertexArrays(1, &mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

DiffusionCurveRenderer::Quad::~Quad()
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

void DiffusionCurveRenderer::Quad::Render()
{
    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
