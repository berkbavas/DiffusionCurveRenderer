#pragma once

#include "Curve/Bezier.h"
#include "Curve/Spline.h"
#include "Util/Macros.h"

#include <QVector>

namespace DiffusionCurveRenderer
{
    class CurveContainer
    {
      public:
        CurveContainer() = default;

        void AddCurve(CurvePtr pCurve);
        void AddCurves(QList<CurvePtr> pCurves);
        void RemoveCurve(CurvePtr pCurve);
        void Clear();

        CurvePtr GetCurve(int iIndex);
        CurvePtr GetCurveAround(const QVector2D& pTest, float fRadius = 8.0f);
        int GetTotalNumberOfCurves() const { return mCurves.size(); }

        float GetGlobalContourThickness() { return mGlobalContourThickness; }
        float GetGlobalDiffusionWidth() { return mGlobalDiffusionWidth; }
        float GetGlobalDiffusionGap() { return mGlobalDiffusionGap; }
        float GetGlobalBlurStrength() { return mGlobalBlurStrength; }

        void SetGlobalContourThickness(float Value);
        void SetGlobalDiffusionWidth(float Value);
        void SetGlobalDiffusionGap(float Value);
        void SetGlobalBlurStrength(float Value);

      private:
        DEFINE_MEMBER(QList<CurvePtr>, Curves);

        float mGlobalContourThickness{ DEFAULT_CONTOUR_THICKNESS };
        float mGlobalDiffusionWidth{ DEFAULT_DIFFUSION_WIDTH };
        float mGlobalDiffusionGap{ DEFAULT_DIFFUSION_GAP };
        float mGlobalBlurStrength{ DEFAULT_BLUR_STRENGTH };
    };
}