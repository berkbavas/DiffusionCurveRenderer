#pragma once

#include <QObject>

namespace DiffusionCurveRenderer
{
    class VectorizationStageBase : public QObject
    {
        Q_OBJECT
      public:
        explicit VectorizationStageBase(QObject* pParent);

        virtual void Reset() = 0;

      signals:
        void Finished();
        void ProgressChanged(float Fraction);
    };
}