#pragma once

#include "Structs/Enums.h"
#include "Util/Logger.h"
#include "Util/Macros.h"
#include "Vectorization/States/ColorSampler.h"
#include "Vectorization/States/CurveConstructor.h"
#include "Vectorization/States/EdgeStack.h"
#include "Vectorization/States/EdgeTracer.h"
#include "Vectorization/States/Potrace.h"

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
        void Vectorize(int edgeLevel);

        cv::Mat GetGaussianStackLayer(int index) { return mGaussianStack.GetLayer(index); }
        cv::Mat GetEdgeStackLayer(int index) { return mEdgeStack.GetLayer(index); }

      signals:
        void ImageLoaded(cv::Mat image);
        void VectorizationStateChanged(VectorizationState state);
        void ProgressChanged(float fraction);
        void GaussianStackFinished(int maximumStackIndex);
        void EdgeStackFinished(int maximumStackIndex);
        void VectorizationFinished(const QVector<CurvePtr>& curves);

      private:
        void Reset();
        void Prepare(const QString& path);

        void SetVectorizationState(VectorizationState state);

        DEFINE_MEMBER_CONST(cv::Mat, OriginalImage);

        // Edges
        DEFINE_MEMBER_CONST(cv::Mat, CannyEdges);
        float mCannyUpperThreshold{ 200.0f };
        float mCannyLowerThreshold{ 20.0f };

        VectorizationState mVectorizationState{ VectorizationState::Ready };

        GaussianStack mGaussianStack;
        EdgeStack mEdgeStack;
        EdgeTracer mEdgeTracer;
        Potrace mPotrace;
        CurveConstructor mCurveConstructor;
        ColorSampler mColorSampler;
    };
}