#pragma once

#include "Structs/Enums.h"
#include "Util/Logger.h"
#include "Util/Macros.h"
#include "Vectorization/Stages/ColorSampler/ColorSampler.h"
#include "Vectorization/Stages/CurveConstructor/BezierCurveConstructor.h"
#include "Vectorization/Stages/CurveConstructor/SplineCurveConstructor.h"
#include "Vectorization/Stages/EdgeStack/EdgeStack.h"
#include "Vectorization/Stages/EdgeTracer/EdgeTracer.h"
#include "Vectorization/Stages/Potrace/Potrace.h"

#include <QObject>
#include <QVariant>
#include <opencv2/core/mat.hpp>

namespace DiffusionCurveRenderer
{
    class BitmapRenderer;

    class VectorizationManager : public QObject
    {
        Q_OBJECT
      public:
        explicit VectorizationManager(QObject* parent = nullptr);

        void LoadImage(const QString& path);
        void Vectorize(VectorizationCurveType curveType, int edgeLevel);

        cv::Mat GetGaussianStackLayer(int index) { return mGaussianStack.GetLayer(index); }
        cv::Mat GetEdgeStackLayer(int index) { return mEdgeStack.GetLayer(index); }

      signals:
        void ImageLoaded(cv::Mat image);
        void ProgressChanged(float fraction);
        void VectorizationStageChanged(VectorizationStage stage);
        void VectorizationStageFinished(VectorizationStage stage, QVariant additionalData = QVariant());
        void VectorizationFinished(const QVector<CurvePtr>& curves);

      private:
        void Setup();
        void Reset();
        void Prepare();

        void SetVectorizationStage(VectorizationStage state);

        DEFINE_MEMBER_CONST(cv::Mat, OriginalImage);

        // Edges
        DEFINE_MEMBER_CONST(cv::Mat, CannyEdges);
        float mCannyUpperThreshold{ 200.0f };
        float mCannyLowerThreshold{ 20.0f };

        VectorizationStage mVectorizationStage{ VectorizationStage::Initial };

        GaussianStack mGaussianStack;
        EdgeStack mEdgeStack;
        EdgeTracer mEdgeTracer;
        Potrace mPotrace;
        SplineCurveConstructor mSplineCurveConstructor;
        BezierCurveConstructor mBezierCurveConstructor;
        ColorSampler mColorSampler;

        CurveConstructor* mCurrentCurveConstructor{ nullptr };
    };
}