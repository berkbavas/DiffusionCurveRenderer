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

        void AddCurve(CurvePtr curve);
        void AddCurves(QList<CurvePtr> curves);
        void RemoveCurve(CurvePtr curve);
        void Clear();

        CurvePtr GetCurve(int index);
        CurvePtr GetCurveAround(const QVector2D& test, float radius = 8.0f);
        int GetTotalNumberOfCurves() const { return mCurves.size(); }

        float GetGlobalContourThickness() { return mGlobalContourThickness; }
        float GetGlobalDiffusionWidth() { return mGlobalDiffusionWidth; }
        float GetGlobalDiffusionGap() { return mGlobalDiffusionGap; }
        float GetGlobalBlurStrength() { return mGlobalBlurStrength; }

        void SetGlobalContourThickness(float val);
        void SetGlobalDiffusionWidth(float val);
        void SetGlobalDiffusionGap(float val);
        void SetGlobalBlurStrength(float val);

      private:
        DEFINE_MEMBER(QList<CurvePtr>, Curves);

        float mGlobalContourThickness{ DEFAULT_CONTOUR_THICKNESS };
        float mGlobalDiffusionWidth{ DEFAULT_DIFFUSION_WIDTH };
        float mGlobalDiffusionGap{ DEFAULT_DIFFUSION_GAP };
        float mGlobalBlurStrength{ DEFAULT_BLUR_STRENGTH };
    };
}