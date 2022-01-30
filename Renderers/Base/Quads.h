#ifndef QUADS_H
#define QUADS_H

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

class Quads : protected QOpenGLFunctions
{
public:
    Quads();
    ~Quads();
    void create();
    void bind();
    void release();

private:
    QOpenGLVertexArrayObject mQuadsVertexArray;
    QOpenGLBuffer mQuadsBuffer;

    static const float VERTICES[24];
};
#endif // QUADS_H
