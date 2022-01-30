#include "Quads.h"

Quads::Quads() {}

Quads::~Quads()
{
    mQuadsVertexArray.destroy();
    mQuadsBuffer.destroy();
}

void Quads::create()
{
    initializeOpenGLFunctions();

    mQuadsVertexArray.create();
    mQuadsVertexArray.bind();

    mQuadsBuffer.create();
    mQuadsBuffer.bind();

    mQuadsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mQuadsBuffer.allocate(&VERTICES, sizeof(VERTICES));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float)));

    mQuadsBuffer.release();
    mQuadsVertexArray.release();
}

void Quads::bind()
{
    mQuadsVertexArray.bind();
}

void Quads::release()
{
    mQuadsVertexArray.release();
}

const float Quads::VERTICES[24] = {-1.0f, 1.0f,  0.0f, 1.0f,

                                   -1.0f, -1.0f, 0.0f, 0.0f,

                                   1.0f,  -1.0f, 1.0f, 0.0f,

                                   -1.0f, 1.0f,  0.0f, 1.0f,

                                   1.0f,  -1.0f, 1.0f, 0.0f,

                                   1.0f,  1.0f,  1.0f, 1.0f};
