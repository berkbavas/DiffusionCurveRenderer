#pragma once

#include <QObject>

namespace DiffusionCurveRenderer
{
    class VectorizationStageBase : public QObject
    {
        Q_OBJECT
      public:
        explicit VectorizationStageBase(QObject* parent);

        virtual void Reset() = 0;

      signals:
        void Finished();
        void ProgressChanged(float fraction);
    };
}