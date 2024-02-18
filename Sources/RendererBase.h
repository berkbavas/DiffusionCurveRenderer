#pragma once

#include "CurveManager.h"
#include "EditModeCamera.h"
#include "Points.h"
#include "Quad.h"
#include "ShaderManager.h"

namespace DiffusionCurveRenderer
{
    class RendererBase : protected QOpenGLExtraFunctions
    {
    public:
        RendererBase();

        virtual void Init();
        virtual void Resize(int w, int h);
        virtual void SetQualityFactor(float newQualityFactor);
        virtual void SetPixelRatio(float newPixelRatio);

        void SetPoints(Points* newPoints);
        void SetQuad(Quad* newQuad);

    protected:
        CurveManager* mCurveManager;
        ShaderManager* mShaderManager;
        EditModeCamera* mCamera;

        Points* mPoints;
        Quad* mQuad;

        float mQualityFactor;
        float mPixelRatio;
        int mWidth;
        int mHeight;
    };
}
