#pragma once

#include <QObject>

namespace DiffusionCurveRenderer
{
    class VectorizationStateBase : public QObject
    {
        Q_OBJECT
      public:
        explicit VectorizationStateBase(QObject* parent);

        virtual void Reset() = 0;

      signals:
        void Finished();
        void ProgressChanged(float fraction);
    };
}