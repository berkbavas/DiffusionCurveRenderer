#pragma once

#include "ColorSampler.h"
#include "CurveManager.h"
#include "EdgeStack.h"
#include "EdgeTracer.h"
#include "GaussianStack.h"
#include "Potrace.h"
#include "BitmapRenderer.h"
#include "CurveConstructor.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QMap>
#include <QObject>
#include <QString>
#include <QThread>

namespace DiffusionCurveRenderer
{
    class Vectorizer : public QObject
    {
        Q_OBJECT
    private:
        explicit Vectorizer(QObject* parent = nullptr);

    public:
        static Vectorizer* Instance();

    public slots:
        void Load(QString path);
        void Draw();

    private:
        void OnLoadDone();
        void OnVectorize();
        void UpdateEdges();

    signals:
        void Vectorize();
        void LoadDone();
        void VectorizationDone();

    private:
        BitmapRenderer* mBitmapRenderer;
        CurveManager* mCurveManager;

        GaussianStack* mGaussianStack;
        EdgeStack* mEdgeStack;
        EdgeTracer* mEdgeTracer;
        Potrace* mPotrace;
        CurveConstructor* mCurveConstructor;
        ColorSampler* mColorSampler;

        float mCannyUpperThreshold;
        float mCannyLowerThreshold;

        cv::Mat mOriginalImage;
        cv::Mat mEdgeImage;
        cv::Mat mLabImage;

        SubWorkMode mSubWorkMode;
        VectorizationStatus mVectorizationStatus;

        // GUI Stuff
        int mSelectedGaussianLayer;
        int mSelectedEdgeLayer;
        bool mInit;
        bool mUpdateData;
    };
}
