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

#include <opencv2/core/mat.hpp>

#include <QObject>
#include <QVariant>

namespace DiffusionCurveRenderer
{
    class VectorizationManager : public QObject
    {
        Q_OBJECT
      public:
        explicit VectorizationManager(QObject* pParent = nullptr);

        void LoadImage(const QString& Path);
        void Vectorize(VectorizationCurveType CurveType, int EdgeLevel);

        cv::Mat GetGaussianStackLayer(int Index) { return mGaussianStack.GetLayer(Index); }
        cv::Mat GetEdgeStackLayer(int Index) { return mEdgeStack.GetLayer(Index); }

      signals:
        void ImageLoaded(cv::Mat Image);
        void ProgressChanged(float Fraction);
        void VectorizationStageChanged(VectorizationStage Stage);
        void VectorizationStageFinished(VectorizationStage Stage, QVariant AdditionalData = QVariant());
        void VectorizationFinished(const QVector<CurvePtr>& Curves);

      private:
        void Setup();
        void Reset();
        void Prepare();
        void SetVectorizationStage(VectorizationStage State);

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