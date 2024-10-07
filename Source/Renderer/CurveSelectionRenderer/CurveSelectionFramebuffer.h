#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QPoint>
#include <QVector4D>
#include <memory>

namespace DiffusionCurveRenderer
{
    struct CurveQueryInfo
    {
        int index;
        int type; // 0: Bezier, 1: Spline
        int unused;
        int result; // 0: Fail, 1: Success
    };

    class CurveSelectionFramebuffer : protected QOpenGLFunctions_4_5_Core
    {
      public:
        CurveSelectionFramebuffer(int width, int height);
        ~CurveSelectionFramebuffer();

        void Clear();
        void Bind();
        CurveQueryInfo Query(const QPoint& queryPoint);

        GLuint GetHandle() const { return mFramebuffer; }
        GLuint GetTexture() const { return mTexture; }

      private:
        GLuint mFramebuffer{ 0 };
        GLuint mTexture{ 0 };

        int mWidth;
        int mHeight;
    };

    using CurveSelectionFramebufferPtr = std::shared_ptr<CurveSelectionFramebuffer>;
}