#pragma once

#include "Core/CurveContainer.h"
#include "Core/OrthographicCamera.h"
#include "Renderer/Base/MultisampleFramebuffer.h"
#include "Renderer/Base/Quad.h"
#include "Renderer/Base/Shader.h"

#include <map>

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>

namespace DiffusionCurveRenderer
{
    class ColorRenderer;
    class DownsampleRenderer;
    class UpsampleRenderer;

    class DiffusionRenderer : protected QOpenGLExtraFunctions
    {
      public:
        DiffusionRenderer() = default;

        void Initialize();
        void Render(QOpenGLFramebufferObject* pTarget = nullptr);
        int GetSmoothIterations() const;
        void SetFramebufferSize(int Size);
        void SetSmoothIterations(int SmoothIterations);
        void SetUseMultisampleFramebuffer(bool Value);

      private:
        ColorRenderer* mColorRenderer;
        DownsampleRenderer* mDownsampleRenderer;
        UpsampleRenderer* mUpsampleRenderer;

        Shader* mBlitter;
        Quad* mQuad;

        QOpenGLFramebufferObjectFormat mFramebufferFormat;
        std::unique_ptr<QOpenGLFramebufferObject> mFramebuffer{ nullptr };

        DEFINE_MEMBER_PTR(OrthographicCamera, Camera);
        DEFINE_MEMBER_PTR(CurveContainer, CurveContainer);
    };
}